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

typedef enum{
	ACT_DO_RESET = 0x1,
	ACT_DO_STOP = 0x2,
} int_action_t;

typedef struct{
	cmd_type_t type;
	uint8_t addr;

	uint8_t *data;
	size_t len;
} int_cmd_t;


/* local/static prototypes */
static size_t cmd_issue(itask_queue_t *queue, cmd_type_t type, uint8_t addr, void *buf, size_t n, avr_i2c_t *i2c);
static void cmd_signal(itask_queue_t *queue);

static int process_int(status_t s, avr_i2c_t *i2c, i2c_regs_t *regs);
static int_action_t process_master_op(status_t s, avr_i2c_t *i2c, i2c_regs_t *regs);
static int_action_t process_slave_op(status_t s, avr_i2c_t *i2c, i2c_regs_t *regs);


/* global functions */
size_t avr_i2c_master_read_int(uint8_t target_addr, uint8_t *buf, size_t n, void *data){
	avr_i2c_t *i2c;


	i2c = (avr_i2c_t*)data;
	n -= cmd_issue(&i2c->master_cmd_queue, CMD_READ, target_addr, buf, n, i2c);

	return n;
}

size_t avr_i2c_master_write_int(uint8_t target_addr, uint8_t *buf, size_t n, void *data){
	avr_i2c_t *i2c;


	i2c = (avr_i2c_t*)data;
	n -= cmd_issue(&i2c->master_cmd_queue, CMD_WRITE, target_addr, buf, n, i2c);

	return n;
}

size_t avr_i2c_slave_read_int(uint8_t *buf, size_t n, void *data){
	return ringbuf_read(&((avr_i2c_t*)data)->slave_rx_buf, buf, n);
}

size_t avr_i2c_slave_write_int(uint8_t *buf, size_t n, void *data){
	avr_i2c_t *i2c;


	i2c = (avr_i2c_t*)data;
	n -= cmd_issue(&i2c->slave_cmd_queue, CMD_WRITE, 0, buf, n, i2c);

	return n;
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

	return cmd.len;
}

static void cmd_signal(itask_queue_t *queue){
	itask_complete(queue);
}

static int process_int(status_t s, avr_i2c_t *i2c, i2c_regs_t *regs){
	int_action_t action;


	action = 0;

	/* handle status */
	switch(s){
	// master mode operations
	case S_NEXT_CMD:
	case S_MST_START:
	case S_MST_RESTART:
	case S_MST_SLAW_NACK:
	case S_MST_SLAR_NACK:
	case S_MST_ARBLOST:
	case S_MST_SLAW_DATA_ACK:
	case S_MST_SLAW_DATA_NACK:
	case S_MST_SLAW_ACK:
	case S_MST_SLAR_DATA_ACK:
	case S_MST_SLAR_DATA_NACK:
	case S_MST_SLAR_ACK:
		action = process_master_op(s, i2c, regs);
		break;

	// slave mode operations
	case S_SLA_SLAW_DATA_ACK:
	case S_SLA_SLAW_DATA_NACK:
	case S_SLA_BCAST_DATA_ACK:
	case S_SLA_BCAST_DATA_NACK:
	case S_SLA_SLAW_ARBLOST_ADDR_MATCH:
	case S_SLA_BCAST_ARBLOST_MATCH:
	case S_SLA_SLAW_MATCH:
	case S_SLA_BCAST_MATCH:
	case S_SLA_SLAR_ARBLOST_ADDR_MATCH:
	case S_SLA_SLAR_ADDR_MATCH:
	case S_SLA_SLAR_DATA_ACK:
	case S_SLA_SLAR_DATA_LAST_ACK:
	case S_SLA_SLAR_DATA_NACK:
	case S_SLA_SLAW_STOP:
		action = process_slave_op(s, i2c, regs);
		break;

	case S_ERROR:
		action = ACT_DO_RESET;
		break;

	case S_INVAL:
	default:
		break;
	}

	/* update hardware state */
	if(action){
		// set hardware to not addressed slave mode
		// potentially trigger a stop command
		regs->twcr = (i2c->dtd->int_num ? 0x1 : 0x0) << TWCR_TWIE
				   | (((action & ACT_DO_STOP) ? 0x1 : 0x0) << TWCR_TWSTO)
				   | (0x1 << TWCR_TWEA)
				   | (0x1 << TWCR_TWEN)
				   | (0x1 << TWCR_TWINT)
				   ;

		return S_NEXT_CMD;
	}

	return S_NONE;
}

static int_action_t process_master_op(status_t s, avr_i2c_t *i2c, i2c_regs_t *regs){
	uint8_t c;
	int_cmd_t *cmd;


	cmd = itask_query_data(&i2c->master_cmd_queue);

	switch(s){
	/* master start */
	case S_NEXT_CMD:
		// start
		if(cmd){
			DEBUG("issue start\n");
			regs->twcr |= (0x1 << TWCR_TWSTA) | (0x1 << TWCR_TWINT);
		}

		break;

	case S_MST_START:
	case S_MST_RESTART:
		DEBUG("start (%#hhx)\n", s);

		// sla-rw
		regs->twdr = cmd->addr << TWDR_ADDR | (cmd->type << TWDR_RW);
		regs->twcr = (i2c->dtd->int_num ? 0x1 : 0x0) << TWCR_TWIE
	   			   | (0x1 << TWCR_TWEN)
				   | (0x1 << TWCR_TWINT)
				   ;
		break;

	case S_MST_SLAW_NACK:
	case S_MST_SLAR_NACK:
		DEBUG("nack (%#hhx)\n", s);

		// stop + start
		regs->twcr |= (0x1 << TWCR_TWSTO) | (0x1 << TWCR_TWSTA) | (0x1 << TWCR_TWINT);
		break;

	case S_MST_ARBLOST:
		DEBUG("arb lost (%#hhx)\n", s);

		// start
		regs->twcr |= (0x1 << TWCR_TWSTA) | (0x1 << TWCR_TWINT);
		break;

	/* master write */
	case S_MST_SLAW_DATA_ACK:
		DEBUG("write (%#hhx): %c (%#hhx)\n", s, *cmd->data, *cmd->data);

		cmd->len--;
		cmd->data++;

		// fall through
	case S_MST_SLAW_DATA_NACK:
		if(cmd->len == 0 || s == S_MST_SLAW_DATA_NACK){
			cmd_signal(&i2c->master_cmd_queue);
			return ACT_DO_STOP;
		}

		// fall through
	case S_MST_SLAW_ACK:
		DEBUG("sla-w (%#hhx)\n", s);

		// write data
		regs->twdr = *cmd->data;
		regs->twcr |= (0x1 << TWCR_TWINT);
		break;

	/* maste read */
	case S_MST_SLAR_DATA_ACK:
	case S_MST_SLAR_DATA_NACK:
		// read data
		c = regs->twdr;
		DEBUG("read (%#hhx): %c (%#hhx)\n", s, c, c);

		if(c != 0xff){
			*cmd->data = c;

			cmd->len--;
			cmd->data++;
		}

		if(cmd->len == 0 || s == S_MST_SLAR_DATA_NACK || c == 0xff){
			cmd_signal(&i2c->master_cmd_queue);
			return ACT_DO_STOP;
		}

		// fall through
	case S_MST_SLAR_ACK:
		DEBUG("sla-r (%#hhx)\n", s);

		// request data
		if(cmd->len == 1)	regs->twcr = (regs->twcr & ~(0x1 << TWCR_TWEA)) | (0x1 << TWCR_TWINT);
		else				regs->twcr |= (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

		break;

	default:
		WARN("invalid status %#hhx\n", s);
		break;
	}

	return 0;
}

static int_action_t process_slave_op(status_t s, avr_i2c_t *i2c, i2c_regs_t *regs){
	uint8_t c;
	int_cmd_t *cmd;


	cmd = itask_query_data(&i2c->slave_cmd_queue);

	switch(s){
		// NOTE lost arbitration does not require any special treatment since the last master
		// 		command will be continued once the slave transmissions has beem finished

	/* slave read */
	case S_SLA_SLAW_DATA_ACK:
	case S_SLA_SLAW_DATA_NACK:
	case S_SLA_BCAST_DATA_ACK:
	case S_SLA_BCAST_DATA_NACK:
		// read data
		c = regs->twdr;
		DEBUG("read (%#hhx): %c (%#hhx)\n", s, c, c);
		ringbuf_write(&i2c->slave_rx_buf, &c, 1);

		if(s == S_SLA_SLAW_DATA_NACK || s == S_SLA_BCAST_DATA_NACK)
			return ACT_DO_RESET;

		// fall through
	case S_SLA_SLAW_ARBLOST_ADDR_MATCH:
	case S_SLA_BCAST_ARBLOST_MATCH:
	case S_SLA_SLAW_MATCH:
	case S_SLA_BCAST_MATCH:
		DEBUG("match (%#hhx)\n", s);

		// ready for data
		if(ringbuf_left(&i2c->slave_rx_buf) == 1)
			regs->twcr = (regs->twcr & ~(0x1 << TWCR_TWEA)) | (0x1 << TWCR_TWINT);
		else
			regs->twcr |= (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

		break;

	/* slave write */
	case S_SLA_SLAR_DATA_ACK:
	case S_SLA_SLAR_DATA_NACK:
	case S_SLA_SLAR_DATA_LAST_ACK:
		if(cmd){
			DEBUG("write (%#hhx): %c (%#hhx)\n", s, *cmd->data, *cmd->data);

			cmd->len--;
			cmd->data++;

			if(cmd->len == 0)
				cmd_signal(&i2c->slave_cmd_queue);
		}

		if(s == S_SLA_SLAR_DATA_LAST_ACK || s == S_SLA_SLAR_DATA_NACK)
			return ACT_DO_RESET;

		// fall through
	case S_SLA_SLAR_ARBLOST_ADDR_MATCH:
	case S_SLA_SLAR_ADDR_MATCH:
		DEBUG("match (%#hhx)\n", s);

		// check for new cmd
		cmd = itask_query_data(&i2c->slave_cmd_queue);

		// send data
		if(cmd)	regs->twdr = *cmd->data;
		else	regs->twdr = 0xff;

		if(cmd && cmd->len > 1)	regs->twcr |= (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);
		else					regs->twcr = (regs->twcr & ~(0x1 << TWCR_TWEA)) | (0x1 << TWCR_TWINT);

		break;

	case S_SLA_SLAW_STOP:
		DEBUG("stop (%#hhx)\n", s);
		return ACT_DO_RESET;

	default:
		WARN("invalid status %#hhx\n", s);
		break;
	}

	return 0;
}
