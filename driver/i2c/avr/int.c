/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/kprintf.h>
#include <kernel/inttask.h>
#include <sys/types.h>
#include <sys/mutex.h>
#include <sys/ringbuf.h>
#include "itf.h"


/* types */
typedef enum{
	CMD_WRITE = 0,
	CMD_READ = 1
} cmd_type_t;

typedef struct{
	cmd_type_t type;
	uint8_t addr;

	uint8_t *data;
	size_t len;
} int_cmd_t;


/* local/static prototypes */
static size_t cmd_issue(itask_queue_t *queue, cmd_type_t type, uint8_t addr, void *buf, size_t n, avr_i2c_t *i2c);
static void cmd_complete(itask_queue_t *queue);

static int process_int(status_t s, avr_i2c_t *i2c, i2c_regs_t *regs);


/* global functions */
size_t avr_i2c_master_read_int(uint8_t target_addr, uint8_t *buf, size_t n, void *data){
	avr_i2c_t *i2c;


	i2c = (avr_i2c_t*)data;
	return cmd_issue(&i2c->master_cmd_queue, CMD_READ, target_addr, buf, n, i2c);
}

size_t avr_i2c_master_write_int(uint8_t target_addr, uint8_t *buf, size_t n, void *data){
	avr_i2c_t *i2c;


	i2c = (avr_i2c_t*)data;
	return cmd_issue(&i2c->master_cmd_queue, CMD_WRITE, target_addr, buf, n, i2c);
}

size_t avr_i2c_slave_read_int(uint8_t *buf, size_t n, void *data){
	return ringbuf_read(&((avr_i2c_t*)data)->slave_rx_buf, buf, n);
}

size_t avr_i2c_slave_write_int(uint8_t *buf, size_t n, void *data){
	avr_i2c_t *i2c;


	i2c = (avr_i2c_t*)data;
	return cmd_issue(&i2c->slave_cmd_queue, CMD_WRITE, 0, buf, n, i2c);
}

void avr_i2c_int_hdlr(int_num_t num, void *data){
	avr_i2c_t *i2c;
	i2c_regs_t *regs;
	status_t s;


	i2c = (avr_i2c_t*)data;
	regs = i2c->dtd->regs;

	mutex_lock(&i2c->mtx);

	s = STATUS(regs);

	// check if the interrupt is triggered by hard- or software
	// software interrupts can be triggered by itask_issue()
	if(!(regs->twcr & (0x1 << TWCR_TWINT)))
		s = S_NEXT_CMD;

	while(s != S_NONE){
		s = process_int(s, i2c, regs);
	}

	mutex_unlock(&i2c->mtx);
}


/* local functions */
static size_t cmd_issue(itask_queue_t *queue, cmd_type_t type, uint8_t addr, void *buf, size_t n, avr_i2c_t *i2c){
	int_cmd_t cmd;


	cmd.type = type;
	cmd.addr = addr;
	cmd.data = buf;
	cmd.len = n;

	itask_issue(queue, &cmd, i2c->dtd->int_num);

	return n - cmd.len;
}

static void cmd_complete(itask_queue_t *queue){
	itask_complete(queue);
}

static int process_int(status_t s, avr_i2c_t *i2c, i2c_regs_t *regs){
	uint8_t c;
	bool stop;
	int_cmd_t *mst_cmd,
			  *slv_cmd;


	stop = false;
	mst_cmd = itask_query_data(&i2c->master_cmd_queue);
	slv_cmd = itask_query_data(&i2c->slave_cmd_queue);

	/* handle status */
	switch(s){
	/* master start */
	case S_NEXT_CMD:
		if(mst_cmd == 0x0)
			return S_NONE;

		DEBUG("issue start\n");
		START(regs);
		break;

	case S_MST_START:
	case S_MST_RESTART:
		DEBUG("start (%#hhx)\n", s);
		SLAVE_RW(regs, mst_cmd->addr, mst_cmd->type, true);
		break;

	case S_MST_SLAW_NACK:
	case S_MST_SLAR_NACK:
		DEBUG("nack (%#hhx)\n", s);
		STOP_START(regs);
		break;

	case S_MST_ARBLOST:
		DEBUG("arb lost (%#hhx)\n", s);
		START(regs);
		break;


	/* master write */
	case S_MST_SLAW_DATA_ACK:
		DEBUG("write (%#hhx): %c (%#hhx)\n", s, *mst_cmd->data, *mst_cmd->data);

		mst_cmd->len--;
		mst_cmd->data++;

		// fall through
	case S_MST_SLAW_DATA_NACK:
		if(mst_cmd->len == 0 || s == S_MST_SLAW_DATA_NACK){
			cmd_complete(&i2c->master_cmd_queue);
			goto do_stop;
		}

		// fall through
	case S_MST_SLAW_ACK:
		DEBUG("sla-w (%#hhx)\n", s);
		DATA_WRITE(regs, *mst_cmd->data, true);
		break;


	/* maste read */
	case S_MST_SLAR_DATA_ACK:
	case S_MST_SLAR_DATA_NACK:
		// read data
		c = DATA_READ(regs);;
		DEBUG("read (%#hhx): %c (%#hhx)\n", s, c, c);

		if(c != 0xff){
			*mst_cmd->data = c;

			mst_cmd->len--;
			mst_cmd->data++;
		}

		if(mst_cmd->len == 0 || s == S_MST_SLAR_DATA_NACK || c == 0xff){
			cmd_complete(&i2c->master_cmd_queue);
			goto do_stop;
		}

		// fall through
	case S_MST_SLAR_ACK:
		DEBUG("sla-r (%#hhx)\n", s);
		REQUEST_DATA(regs, (mst_cmd->len > 1));
		break;


	/* slave read */
	case S_SLA_SLAW_DATA_ACK:
	case S_SLA_SLAW_DATA_NACK:
	case S_SLA_BCAST_DATA_ACK:
	case S_SLA_BCAST_DATA_NACK:
		// read data
		c = DATA_READ(regs);
		DEBUG("read (%#hhx): %c (%#hhx)\n", s, c, c);

		ringbuf_write(&i2c->slave_rx_buf, &c, 1);

		if(s == S_SLA_SLAW_DATA_NACK || s == S_SLA_BCAST_DATA_NACK)
			goto do_reset;

		// fall through
	// NOTE lost arbitration does not require any special treatment since the last master
	// 		command will be continued once the slave transmissions has beem finished
	case S_SLA_SLAW_ARBLOST_ADDR_MATCH:
	case S_SLA_BCAST_ARBLOST_MATCH:
	case S_SLA_SLAW_MATCH:
	case S_SLA_BCAST_MATCH:
		DEBUG("match (%#hhx)\n", s);
		REQUEST_DATA(regs, (ringbuf_left(&i2c->slave_rx_buf) >= 1));
		break;


	/* slave write */
	case S_SLA_SLAR_DATA_ACK:
	case S_SLA_SLAR_DATA_NACK:
	case S_SLA_SLAR_DATA_LAST_ACK:
		if(slv_cmd){
			DEBUG("write (%#hhx): %c (%#hhx)\n", s, *slv_cmd->data, *slv_cmd->data);

			slv_cmd->len--;
			slv_cmd->data++;

			if(slv_cmd->len == 0)
				cmd_complete(&i2c->slave_cmd_queue);
		}

		if(s == S_SLA_SLAR_DATA_LAST_ACK || s == S_SLA_SLAR_DATA_NACK)
			goto do_reset;

		// fall through
	// NOTE lost arbitration does not require any special treatment since the last master
	// 		command will be continued once the slave transmissions has beem finished
	case S_SLA_SLAR_ARBLOST_ADDR_MATCH:
	case S_SLA_SLAR_ADDR_MATCH:
		DEBUG("match (%#hhx)\n", s);

		slv_cmd = itask_query_data(&i2c->slave_cmd_queue);	// last cmd might be finished
		DATA_WRITE(regs, (slv_cmd ? *slv_cmd->data : 0xff), (slv_cmd && slv_cmd->len > 1));
		break;

	case S_SLA_SLAW_STOP:
		DEBUG("stop (%#hhx)\n", s);
		goto do_reset;


	/* misc states */
	case S_ERROR:
		goto do_reset;

	case S_INVAL:
	default:
		WARN("invalid status %#hhx\n", s);
		break;
	}

	return S_NONE;


	/* reset hardware */
do_stop:
	stop = true;

do_reset:
	SET_SLAVE_MODE(regs, stop, true);

	return S_NEXT_CMD;
}
