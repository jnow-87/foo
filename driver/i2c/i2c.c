/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>
#include <kernel/interrupt.h>
#include <kernel/inttask.h>
#include <driver/i2c.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mutex.h>
#include <sys/string.h>
#include <sys/ringbuf.h>


/* local/static prototypes */
static size_t rw(i2c_t *i2c, i2c_cmd_t cmd, uint8_t slave, void *buf, size_t n);

static size_t poll_cmd(i2c_t *i2c, i2c_dgram_t *dgram);
static size_t int_cmd(i2c_t *i2c, i2c_dgram_t *dgram);

static void int_hdlr(int_num_t num, void *i2c);

static int int_master(i2c_t *i2c, i2c_dgram_t *dgram, i2c_state_t state);
static int int_slave(i2c_t *i2c, i2c_dgram_t *dgram, i2c_state_t state);

static int complete(i2c_t *i2c, errno_t ecode, bool stop);


/* global functions */
i2c_t *i2c_create(i2c_ops_t *ops, i2c_cfg_t *cfg, void *hw){
	i2c_t *i2c;


	i2c = kcalloc(1, sizeof(i2c_t));

	if(i2c == 0x0)
		goto err_0;

	i2c->cfg = cfg;
	i2c->hw = hw;

	memcpy(&i2c->ops, ops, sizeof(i2c_ops_t));

	mutex_init(&i2c->mtx, MTX_NOINT);
	itask_queue_init(&i2c->master_cmds);
	itask_queue_init(&i2c->slave_cmds);

	if(ops->configure(i2c->cfg, i2c->hw) != 0)
		goto err_1;

	ops->slave_mode(false, false, hw);

	if(cfg->int_num && int_register(cfg->int_num, int_hdlr, i2c) != 0)
		goto err_1;

	return i2c;


err_1:
	i2c_destroy(i2c);

err_0:
	return 0x0;
}

void i2c_destroy(i2c_t *i2c){
	if(i2c->cfg->int_num)
		int_release(i2c->cfg->int_num);

	itask_queue_destroy(&i2c->master_cmds);
	itask_queue_destroy(&i2c->slave_cmds);

	kfree(i2c);
}

size_t i2c_read(i2c_t *i2c, uint8_t slave, void *buf, size_t n){
	n = rw(i2c, I2C_CMD_READ, slave, buf, n);

	// errors are tolerated if some bytes are transfered
	if(errno && n > 0)
		set_errno(E_OK);

	return n;
}

size_t i2c_write(i2c_t *i2c, uint8_t slave, void *buf, size_t n){
	size_t r;


	r = rw(i2c, I2C_CMD_WRITE, slave, buf, n);

	// errors are tolerated if some bytes are transfered
	if(errno && n != r)
		set_errno(E_OK);

	return n - r;
}

/* local functions */
static size_t rw(i2c_t *i2c, i2c_cmd_t cmd, uint8_t slave, void *buf, size_t n){
	i2c_dgram_t dgram;


	dgram.cmd = cmd;
	dgram.slave = slave;

	linebuf_init(&dgram.data, buf, n, (cmd == I2C_CMD_WRITE) ? n : 0);

	return (i2c->cfg->int_num ? int_cmd(i2c, &dgram) : poll_cmd(i2c, &dgram));
}

static size_t poll_cmd(i2c_t *i2c, i2c_dgram_t *dgram){
	int r;
	i2c_state_t state;
	int (*op)(i2c_t *, i2c_dgram_t *, i2c_state_t);


	state = I2C_STATE_NEXT_CMD;
	op = (i2c->cfg->mode == I2C_MODE_SLAVE) ? int_slave : int_master;

	while(1){
		mutex_lock(&i2c->mtx);

		r = op(i2c, dgram, state);
		state = i2c->ops.state(i2c->hw);

		mutex_unlock(&i2c->mtx);

		if(r <= 0)
			goto_errno(end, -r);
	}

end:
	return linebuf_contains(&dgram->data);
}

static size_t int_cmd(i2c_t *i2c, i2c_dgram_t *dgram){
	itask_queue_t *queue;


	queue = (i2c->cfg->mode == I2C_MODE_MASTER) ? &i2c->master_cmds : &i2c->slave_cmds;
	itask_issue(queue, dgram, i2c->cfg->int_num);

	return linebuf_contains(&dgram->data);
}

static void int_hdlr(int_num_t num, void *_i2c){
	int r;
	bool master_cmd;
	i2c_t *i2c;
	i2c_state_t state;
	i2c_dgram_t *dgram;
	itask_queue_t *cmds;


	i2c = (i2c_t*)_i2c;

	mutex_lock(&i2c->mtx);

	/* identify dgram */
	state = i2c->ops.state(i2c->hw);

	master_cmd = (i2c->cfg->mode == I2C_MODE_MASTER && !(state & I2C_STATE_BIT_SLAVE));
	cmds = master_cmd ? &i2c->master_cmds : &i2c->slave_cmds;
	dgram = itask_query_data(cmds, 0x0);

	if(dgram == 0x0)
		goto unlock;

	/* perform state action */
	// ensure master commands are started
	if(state == I2C_STATE_NONE)
		state = I2C_STATE_NEXT_CMD;

	r = master_cmd ? int_master(i2c, dgram, state) : int_slave(i2c, dgram, state);

	if(r > 0)
		goto unlock;

	/* transfer complete */
	itask_complete(cmds, -r);

	if(itask_query_data(&i2c->master_cmds, 0x0) || itask_query_data(&i2c->slave_cmds, 0x0))
		int_foretell(i2c->cfg->int_num);

unlock:
	mutex_unlock(&i2c->mtx);
}

static int int_master(i2c_t *i2c, i2c_dgram_t *dgram, i2c_state_t state){
	uint8_t c;
	i2c_ops_t *ops;


	ops = &i2c->ops;

	switch(state){
	/* master start */
	case I2C_STATE_MST_ARBLOST:
		DEBUG("arb lost (%#hhx)\n", state);

		// fall through
	case I2C_STATE_NEXT_CMD:
		DEBUG("issue start\n");
		ops->start(i2c->hw);
		break;

	case I2C_STATE_MST_START:
	case I2C_STATE_MST_RESTART:
		DEBUG("start (%#hhx)\n", state);
		ops->slave_addr(dgram->cmd, dgram->slave, i2c->hw);
		break;

	case I2C_STATE_MST_SLAW_NACK:
	case I2C_STATE_MST_SLAR_NACK:
		DEBUG("nack (%#hhx)\n", state);
		return complete(i2c, E_NOCONN, true);


	/* master write */
	case I2C_STATE_MST_SLAW_DATA_ACK:
		linebuf_read(&dgram->data, &c, 1);

		// fall through
	case I2C_STATE_MST_SLAW_DATA_NACK:
		DEBUG("(n)ack (%#hhx)\n", state);

		if(linebuf_empty(&dgram->data) || state == I2C_STATE_MST_SLAW_DATA_NACK)
			return complete(i2c, E_OK, true);

		// fall through
	case I2C_STATE_MST_SLAW_ACK:
		linebuf_peek(&dgram->data, &c, 1);
		DEBUG("sla-w (%#hhx): %#hhx\n", state, c);

		ops->byte_write(c, true, i2c->hw);
		break;


	/* master read */
	case I2C_STATE_MST_SLAR_DATA_ACK:
	case I2C_STATE_MST_SLAR_DATA_NACK:
		c = ops->byte_read(i2c->hw);
		linebuf_write(&dgram->data, &c, 1);
		DEBUG("read (%#hhx): %#hhx\n", state, c);

		if(linebuf_empty(&dgram->data) || state == I2C_STATE_MST_SLAR_DATA_NACK)
			return complete(i2c, E_OK, true);

		// fall through
	case I2C_STATE_MST_SLAR_ACK:
		DEBUG("sla-r (%#hhx)\n", state);
		ops->ack((linebuf_left(&dgram->data) > 1), i2c->hw);
		break;


	/* no state change */
	case I2C_STATE_NONE:
		break;

	/* error */
	case I2C_STATE_ERROR:
	case I2C_STATE_INVAL:
	default:
		return complete(i2c, E_IO, true);
	}

	return 1;
}

static int int_slave(i2c_t *i2c, i2c_dgram_t *dgram, i2c_state_t state){
	uint8_t c;
	i2c_ops_t *ops;


	ops = &i2c->ops;

	switch(state){
	case I2C_STATE_NEXT_CMD:
		DEBUG("make addressable\n");
		i2c->ops.slave_mode(true, false, i2c->hw);
		break;

	/* slave read */
	case I2C_STATE_SLA_SLAW_DATA_ACK:
	case I2C_STATE_SLA_SLAW_DATA_NACK:
	case I2C_STATE_SLA_BCAST_DATA_ACK:
	case I2C_STATE_SLA_BCAST_DATA_NACK:
		c = ops->byte_read(i2c->hw);
		linebuf_write(&dgram->data, &c, 1);
		DEBUG("read (%#hhx): %#hhx\n", state, c);

		if(state == I2C_STATE_SLA_SLAW_DATA_NACK || state == I2C_STATE_SLA_BCAST_DATA_NACK)
			return complete(i2c, E_OK, false);

		// fall through
	// NOTE lost arbitration does not require any special treatment since the last master
	// 		command will be continued once the slave transmission has beem finished
	case I2C_STATE_SLA_SLAW_ARBLOST_ADDR_MATCH:
	case I2C_STATE_SLA_BCAST_ARBLOST_MATCH:
	case I2C_STATE_SLA_SLAW_MATCH:
	case I2C_STATE_SLA_BCAST_MATCH:
		DEBUG("match (%#hhx)\n", state);
		ops->ack(!(linebuf_full(&dgram->data)), i2c->hw);
		break;


	/* slave write */
	case I2C_STATE_SLA_SLAR_DATA_ACK:
	case I2C_STATE_SLA_SLAR_DATA_NACK:
	case I2C_STATE_SLA_SLAR_DATA_LAST_ACK:
		DEBUG("(n)ack (%#hhx)\n", state);

		if(linebuf_empty(&dgram->data)){
			itask_complete(&i2c->slave_cmds, E_OK);
			dgram = itask_query_data(&i2c->slave_cmds, 0x0);
		}

		if(state == I2C_STATE_SLA_SLAR_DATA_LAST_ACK || state == I2C_STATE_SLA_SLAR_DATA_NACK || dgram == 0x0)
			return complete(i2c, E_OK, false);

		// fall through
	// NOTE lost arbitration does not require any special treatment since the last master
	// 		command will be continued once the slave transmission has beem finished
	case I2C_STATE_SLA_SLAR_ARBLOST_ADDR_MATCH:
	case I2C_STATE_SLA_SLAR_ADDR_MATCH:
		linebuf_read(&dgram->data, &c, 1);
		DEBUG("write (%#hhx): %#hhx\n", state, c);

		ops->byte_write(c, (linebuf_contains(&dgram->data) == 0), i2c->hw);
		break;

	case I2C_STATE_SLA_SLAW_STOP:
		DEBUG("stop (%#hhx)\n", state);
		return complete(i2c, E_OK, false);


	/* no state change */
	case I2C_STATE_NONE:
		break;

	/* error */
	case I2C_STATE_ERROR:
	case I2C_STATE_INVAL:
	default:
		return complete(i2c, E_IO, true);
	}

	return 1;
}

static int complete(i2c_t *i2c, errno_t ecode, bool stop){
	DEBUG("complete: %s, %sstop\n", strerror(ecode), (stop ? "" : "no "));
	i2c->ops.slave_mode(false, stop, i2c->hw);

	return -ecode;
}
