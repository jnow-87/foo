/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/kprintf.h>
#include <driver/i2c.h>
#include <sys/types.h>
#include <sys/errno.h>


/* types */
typedef struct{
	i2c_cmd_t cmd;
	uint8_t remote;

	uint8_t *data;
	size_t len;
} cmd_data_t;


/* local/static prototypes */
static i2c_state_t int_master(i2c_state_t state, i2c_int_data_t *data, i2c_primitives_t *prim);
static i2c_state_t int_slave(i2c_state_t state, i2c_int_data_t *data, i2c_primitives_t *prim);
static i2c_state_t int_special(i2c_state_t state, i2c_int_data_t *data, i2c_primitives_t *prim);

static i2c_state_t complete_task(itask_queue_t *queue, int err, bool stop, i2c_primitives_t *prim);
static i2c_state_t reset(bool stop, i2c_primitives_t *prim);


/* global functions */
int i2c_int_data_init(i2c_int_data_t *data){
	void *buf;


	buf = kmalloc(CONFIG_I2C_RXBUF_SIZE);

	if(buf == 0x0)
		return -errno;

	itask_queue_init(&data->master_cmds);
	itask_queue_init(&data->slave_cmds);
	ringbuf_init(&data->rx_buf, buf, CONFIG_I2C_RXBUF_SIZE);

	return E_OK;
}

size_t i2c_int_cmd(i2c_cmd_t cmd, i2c_int_data_t *data, uint8_t remote, void *buf, size_t n, i2c_primitives_t *prim){
	cmd_data_t task;
	itask_queue_t *queue;


	if(cmd == (I2C_CMD_SLAVE | I2C_CMD_READ)){
		critsec_lock(&data->lock);
		n = ringbuf_read(&data->rx_buf, buf, n);
		critsec_unlock(&data->lock);

		return n;
	}

	task.cmd = cmd;
	task.remote = remote;
	task.data = buf;
	task.len = n;

	queue = (cmd & I2C_CMD_MASTER) ? &data->master_cmds : &data->slave_cmds;
	(void)itask_issue(queue, &task, prim->int_num);

	return n - task.len;
}

void i2c_int_hdlr(i2c_int_data_t *data, i2c_primitives_t *prim){
	i2c_state_t s;


	s = prim->state(false, prim->data);

	while(s != I2C_STATE_NONE){
		if(s & I2C_STATE_BIT_MASTER)		s = int_master(s, data, prim);
		else if(s & I2C_STATE_BIT_SLAVE)	s = int_slave(s, data, prim);
		else								s = int_special(s, data, prim);
	}
}


/* local functions */
static i2c_state_t int_master(i2c_state_t state, i2c_int_data_t *data, i2c_primitives_t *prim){
	uint8_t c;
	cmd_data_t *cmd;


	cmd = itask_query_data(&data->master_cmds);

	if(cmd == 0x0)
		return I2C_STATE_NONE;

	switch(state){
	/* master start */
	case I2C_STATE_MST_NOINT:
	case I2C_STATE_MST_NEXT_CMD:
		DEBUG("issue start\n");
		prim->start(prim->data);
		break;

	case I2C_STATE_MST_START:
	case I2C_STATE_MST_RESTART:
		DEBUG("start (%#hhx)\n", state);
		prim->slave_read_write(cmd->remote, cmd->cmd & I2C_CMD_READ, true, prim->data);
		break;

	case I2C_STATE_MST_SLAW_NACK:
	case I2C_STATE_MST_SLAR_NACK:
		DEBUG("nack (%#hhx)\n", state);
		return complete_task(&data->master_cmds, E_NOCONN, true, prim);

	case I2C_STATE_MST_ARBLOST:
		DEBUG("arb lost (%#hhx)\n", state);
		prim->start(prim->data);
		break;


	/* master write */
	case I2C_STATE_MST_SLAW_DATA_ACK:
		DEBUG("write (%#hhx): %c (%#hhx)\n", state, *cmd->data, *cmd->data);

		cmd->len--;
		cmd->data++;

		// fall through
	case I2C_STATE_MST_SLAW_DATA_NACK:
		if(cmd->len == 0 || state == I2C_STATE_MST_SLAW_DATA_NACK)
			return complete_task(&data->master_cmds, E_OK, true, prim);

		// fall through
	case I2C_STATE_MST_SLAW_ACK:
		DEBUG("sla-w (%#hhx)\n", state);
		prim->byte_write(*cmd->data, true, prim->data);
		break;


	/* maste read */
	case I2C_STATE_MST_SLAR_DATA_ACK:
	case I2C_STATE_MST_SLAR_DATA_NACK:
		// read data
		c = prim->byte_read(prim->data);
		DEBUG("read (%#hhx): %c (%#hhx)\n", state, c, c);

		if(c != 0xff){
			*cmd->data = c;

			cmd->len--;
			cmd->data++;
		}

		if(cmd->len == 0 || state == I2C_STATE_MST_SLAR_DATA_NACK || c == 0xff)
			return complete_task(&data->master_cmds, E_OK, true, prim);

		// fall through
	case I2C_STATE_MST_SLAR_ACK:
		DEBUG("sla-r (%#hhx)\n", state);
		prim->byte_request((cmd->len > 1), prim->data);
		break;

	default:
		WARN("invalid state %#hhx\n", state);
		break;
	}

	return I2C_STATE_NONE;
}

static i2c_state_t int_slave(i2c_state_t state, i2c_int_data_t *data, i2c_primitives_t *prim){
	uint8_t c;
	cmd_data_t *cmd;


	cmd = itask_query_data(&data->slave_cmds);

	switch(state){
	/* slave read */
	case I2C_STATE_SLA_SLAW_DATA_ACK:
	case I2C_STATE_SLA_SLAW_DATA_NACK:
	case I2C_STATE_SLA_BCAST_DATA_ACK:
	case I2C_STATE_SLA_BCAST_DATA_NACK:
		// read data
		c = prim->byte_read(prim->data);
		DEBUG("read (%#hhx): %c (%#hhx)\n", state, c, c);

		ringbuf_write(&data->rx_buf, &c, 1);

		if(state == I2C_STATE_SLA_SLAW_DATA_NACK || state == I2C_STATE_SLA_BCAST_DATA_NACK)
			return reset(false, prim);

		// fall through
	// NOTE lost arbitration does not require any special treatment since the last master
	// 		command will be continued once the slave transmissions has beem finished
	case I2C_STATE_SLA_SLAW_ARBLOST_ADDR_MATCH:
	case I2C_STATE_SLA_BCAST_ARBLOST_MATCH:
	case I2C_STATE_SLA_SLAW_MATCH:
	case I2C_STATE_SLA_BCAST_MATCH:
		DEBUG("match (%#hhx)\n", state);
		prim->byte_request((ringbuf_left(&data->rx_buf) >= 1), prim->data);
		break;


	/* slave write */
	case I2C_STATE_SLA_SLAR_DATA_ACK:
	case I2C_STATE_SLA_SLAR_DATA_NACK:
	case I2C_STATE_SLA_SLAR_DATA_LAST_ACK:
		if(cmd){
			DEBUG("write (%#hhx): %c (%#hhx)\n", state, *cmd->data, *cmd->data);

			cmd->len--;
			cmd->data++;

			if(cmd->len == 0)
				itask_complete(&data->slave_cmds, E_OK);
		}

		if(state == I2C_STATE_SLA_SLAR_DATA_LAST_ACK || state == I2C_STATE_SLA_SLAR_DATA_NACK)
			return reset(false, prim);

		// fall through
	// NOTE lost arbitration does not require any special treatment since the last master
	// 		command will be continued once the slave transmissions has beem finished
	case I2C_STATE_SLA_SLAR_ARBLOST_ADDR_MATCH:
	case I2C_STATE_SLA_SLAR_ADDR_MATCH:
		DEBUG("match (%#hhx)\n", state);

		cmd = itask_query_data(&data->slave_cmds);	// last cmd might be finished
		prim->byte_write((cmd ? *cmd->data : 0xff), (cmd && cmd->len > 1), prim->data);
		break;

	case I2C_STATE_SLA_SLAW_STOP:
		DEBUG("stop (%#hhx)\n", state);
		return reset(false, prim);

	default:
		WARN("invalid state %#hhx\n", state);
		break;
	}

	return I2C_STATE_NONE;
}

static i2c_state_t int_special(i2c_state_t state, i2c_int_data_t *data, i2c_primitives_t *prim){
	switch(state){
	case I2C_STATE_ERROR:
		if(itask_query_data(&data->master_cmds) != 0x0)
			return complete_task(&data->master_cmds, E_IO, true, prim);

		if(itask_query_data(&data->slave_cmds) != 0x0)
			return complete_task(&data->slave_cmds, E_IO, false, prim);

		return reset(false, prim);

	case I2C_STATE_INVAL:
	default:
		WARN("invalid state %#hhx\n", state);
		break;
	}

	return I2C_STATE_NONE;
}

static i2c_state_t complete_task(itask_queue_t *queue, int err, bool stop, i2c_primitives_t *prim){
	itask_complete(queue, err);

	return reset(stop, prim);
}

static i2c_state_t reset(bool stop, i2c_primitives_t *prim){
	prim->slave_mode_set(stop, true, prim->data);

	return I2C_STATE_MST_NEXT_CMD;
}
