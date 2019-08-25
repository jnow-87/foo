/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/interrupt.h>
#include <arch/avr/register.h>
#include <kernel/driver.h>
#include <kernel/kprintf.h>
#include <kernel/sigqueue.h>
#include <driver/i2c.h>
#include <sys/ringbuf.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mutex.h>


/* macros */
// register bits
#define TWCR_TWINT		7
#define TWCR_TWEA		6
#define TWCR_TWSTA		5
#define TWCR_TWSTO		4
#define TWCR_TWWC		3
#define TWCR_TWEN		2
#define TWCR_TWIE		0

#define TWSR_TWS		3
#define TWSR_TWPS		0

#define TWBR_BR			0

#define TWDR_ADDR		1
#define TWDR_RW			0

#define TWAR_TWA		1
#define TWAR_TWGCE		0

#define TWAMR_TWAM		1

// instructions
#define STATUS(regs) \
	(regs)->dev->twsr & ~(0x3 << TWSR_TWPS)

#define WAITINT(regs)({ \
	while(!((regs)->dev->twcr & (0x1 << TWCR_TWINT))); \
	STATUS(regs); \
})


/* types */
typedef struct{
	// device registers
	struct{
		uint8_t volatile twbr,
						 twsr,
						 twar,
						 twdr,
						 twcr,
						 twamr;
	} *dev;

	// power control
	uint8_t volatile *prr;
	uint8_t const prr_en;		// PRR device enable value (bit mask)

	uint8_t const int_num;
} dt_data_t;

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

typedef struct{
	i2c_itf_t itf;
	dt_data_t *regs;

	mutex_t mtx;

	sigq_queue_t master_cmd_queue,
				 slave_cmd_queue;
	ringbuf_t slave_rx_buf;
} dev_data_t;

typedef enum{
	// master mode status
	S_MST_START = 0x08,
	S_MST_RESTART = 0x10,

	S_MST_SLAW_ACK = 0x18,
	S_MST_SLAW_NACK = 0x20,
	S_MST_SLAW_DATA_ACK = 0x28,
	S_MST_SLAW_DATA_NACK = 0x30,

	S_MST_SLAR_ACK = 0x40,
	S_MST_SLAR_NACK = 0x48,
	S_MST_SLAR_DATA_ACK = 0x50,
	S_MST_SLAR_DATA_NACK = 0x58,

	S_MST_ARBLOST = 0x38,

	// slave mode status
	S_SLA_SLAW_MATCH = 0x60,
	S_SLA_SLAW_ARBLOST_ADDR_MATCH = 0x68,
	S_SLA_SLAR_ADDR_MATCH = 0xa8,
	S_SLA_SLAR_ARBLOST_ADDR_MATCH = 0xb0,
	S_SLA_BCAST_MATCH = 0x70,
	S_SLA_BCAST_ARBLOST_MATCH = 0x78,

	S_SLA_SLAW_DATA_ACK = 0x80,
	S_SLA_SLAW_DATA_NACK = 0x88,
	S_SLA_BCAST_DATA_ACK = 0x90,
	S_SLA_BCAST_DATA_NACK = 0x98,

	S_SLA_SLAW_STOP = 0xa0,

	S_SLA_SLAR_DATA_ACK = 0xb8,
	S_SLA_SLAR_DATA_NACK = 0xc0,
	S_SLA_SLAR_DATA_LAST_ACK = 0xc8,

	// misc
	S_ERROR = 0x0,
	S_INVAL = 0xf8,
	S_NEXT_CMD = 0x1,	// artificial state introduced by the driver
						// to process the next interrupt command
} status_t;


/* local/static prototypes */
static int configure(i2c_cfg_t *cfg, void *data);
static int master_read(uint8_t target_addr, uint8_t *buf, size_t n, void *data);
static int master_write(uint8_t target_addr, uint8_t *buf, size_t n, void *data);
static int slave_read(uint8_t *buf, size_t n, void *data);
static int slave_write(uint8_t *buf, size_t n, void *data);

static size_t cmd_issue(sigq_queue_t *queue, cmd_type_t type, uint8_t addr, void *buf, size_t n, bool trigger, dev_data_t *i2c);
static void cmd_signal(sigq_queue_t *queue, sigq_t *e);

static void int_hdlr(int_num_t num, void *data);
static void process_int(status_t s, dev_data_t *i2c, dt_data_t *regs);
static int_action_t process_master_op(status_t s, dev_data_t *i2c, dt_data_t *regs);
static int_action_t process_slave_op(status_t s, dev_data_t *i2c, dt_data_t *regs);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	void *buf;
	dev_data_t *i2c;
	dt_data_t *regs;


	regs = (dt_data_t*)dt_data;

	/* allocate rx buffer */
	buf = 0x0;

	if(regs->int_num){
		buf = kmalloc(CONFIG_I2C_RXBUF_SIZE);

		if(buf == 0x0)
			goto err_0;
	}

	/* allocate interface */
	i2c = kmalloc(sizeof(dev_data_t));

	if(i2c == 0x0)
		goto err_1;

	i2c->itf.configure = configure;
	i2c->itf.master_read = master_read;
	i2c->itf.master_write = master_write;
	i2c->itf.slave_read = slave_read;
	i2c->itf.slave_write = slave_write;
	i2c->itf.data = i2c;
	i2c->regs = regs;

	mutex_init(&i2c->mtx, MTX_NONE);
	sigq_queue_init(&i2c->master_cmd_queue);
	sigq_queue_init(&i2c->slave_cmd_queue);
	ringbuf_init(&i2c->slave_rx_buf, buf, CONFIG_I2C_RXBUF_SIZE);

	/* register interrupt */
	if(regs->int_num && int_register(regs->int_num, int_hdlr, i2c) != 0)
		goto err_2;

	return &i2c->itf;


err_2:
	kfree(i2c);

err_1:
	kfree(buf);

err_0:
	return 0x0;
}

interface_probe("avr,i2c", probe);

static int configure(i2c_cfg_t *cfg, void *data){
	uint8_t brate;
	dt_data_t *regs;
	dev_data_t *i2c;


	i2c = (dev_data_t*)data;
	regs = i2c->regs;

	/* check config */
	// max 400 kHz
	if(cfg->clock_khz > 400)
		return_errno(E_LIMIT);

	// only 7-bit addresses
	if(cfg->host_addr & 0x80 || cfg->target_addr & 0x80)
		return_errno(E_LIMIT);

	/* compute baud rate */
	// NOTE assumption: TWSR[TWPS] = 0
	brate = ((AVR_IO_CLOCK_HZ / (cfg->clock_khz * 1000)) - 16) / 2;

	if(brate == 0)
		return_errno(E_INVAL);

	mutex_lock(&i2c->mtx);

	/* disable twi */
	*regs->prr |= regs->prr_en;

	/* re-enable twi */
	*regs->prr &= ~regs->prr_en;

	regs->dev->twsr = 0x0;
	regs->dev->twbr = brate;
	regs->dev->twamr = 0x0;
	regs->dev->twar = cfg->host_addr << 0x1 | ((cfg->bcast_en ? 0x1 : 0x0) << TWAR_TWGCE);
	regs->dev->twcr = (0x1 << TWCR_TWEN)
					| (0x1 << TWCR_TWEA)
					| ((regs->int_num ? 0x1 : 0x0) << TWCR_TWIE)
					;

	mutex_unlock(&i2c->mtx);

	return E_OK;
}

static int master_read(uint8_t target_addr, uint8_t *buf, size_t n, void *data){
	size_t i;
	uint8_t cr,
			c;
	dev_data_t *i2c;
	dt_data_t *regs;
	status_t s;


	i2c = (dev_data_t*)data;
	regs = i2c->regs;
	i = 0;

	if(i2c->regs->int_num){
		n -= cmd_issue(&i2c->master_cmd_queue, CMD_READ, target_addr, buf, n, true, i2c);
		return n;
	}

	mutex_lock(&i2c->mtx);

	cr = (regs->int_num ? 0x1 : 0x0) << TWCR_TWIE
	   | (0x1 << TWCR_TWEN)
	   ;

	/* start */
	regs->dev->twcr = cr | (0x1 << TWCR_TWSTA) | (0x1 << TWCR_TWINT);
	s = WAITINT(regs);

	if(!(s == S_MST_START || s == S_MST_RESTART))
		goto_errno(end, E_IO);

	DEBUG("start (%#hhx)\n", s);

	/* slave read */
	regs->dev->twdr = target_addr << TWDR_ADDR | (0x1 << TWDR_RW);
	regs->dev->twcr = cr | (0x1 << TWCR_TWINT);

	s = WAITINT(regs);

	if(!(s == S_MST_SLAR_ACK))
		goto_errno(end, E_IO);

	DEBUG("sla-r (%#hhx)\n", s);

	/* read bytes */
	while(i < n){
		if(i + 1 == n)	regs->dev->twcr = cr | (0x1 << TWCR_TWINT);
		else			regs->dev->twcr = cr | (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

		s = WAITINT(regs);
		c = regs->dev->twdr;
		DEBUG("read (%#hhx): %c (%#hhx)\n", s, c, c);

		// no data or invalid data (0xff)
		if(!(s == S_MST_SLAR_DATA_ACK || s == S_MST_SLAR_DATA_NACK) || c == 0xff)
			break;

		buf[i++] = c;

		// no further data, i.e. nack
		if(!(s == S_MST_SLAR_DATA_ACK))
			break;
	}

end:
	/* stop and reset to not addressed slave mode */
	regs->dev->twcr = cr | (0x1 << TWCR_TWSTO) | (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

	mutex_unlock(&i2c->mtx);

	return i;
}

static int master_write(uint8_t target_addr, uint8_t *buf, size_t n, void *data){
	size_t i;
	register uint8_t cr;
	dev_data_t *i2c;
	dt_data_t *regs;
	status_t s;


	i2c = (dev_data_t*)data;
	regs = i2c->regs;
	i = 0;

	if(i2c->regs->int_num){
		n -= cmd_issue(&i2c->master_cmd_queue, CMD_WRITE, target_addr, buf, n, true, i2c);
		return n;
	}

	mutex_lock(&i2c->mtx);

	cr = (regs->int_num ? 0x1 : 0x0) << TWCR_TWIE
	   | (0x1 << TWCR_TWEN)
	   ;

	/* start */
	regs->dev->twcr = cr | (0x1 << TWCR_TWSTA) | (0x1 << TWCR_TWINT);
	s = WAITINT(regs);

	if(!(s == S_MST_START || s == S_MST_RESTART))
		goto_errno(end, E_IO);

	DEBUG("start (%#hhx)\n", s);

	/* slave write */
	regs->dev->twdr = target_addr << TWDR_ADDR | (0x0 << TWDR_RW);
	regs->dev->twcr = cr | (0x1 << TWCR_TWINT);

	s = WAITINT(regs);

	if(!(s == S_MST_SLAW_ACK))
		goto_errno(end, E_IO);

	DEBUG("sla-w (%#hhx)\n", s);

	/* write bytes */
	while(i < n){
		regs->dev->twdr = buf[i];
		regs->dev->twcr = cr | (0x1 << TWCR_TWINT);

		s = WAITINT(regs);
		DEBUG("write (%#hhx): %c (%#hhx)\n", s, buf[i], buf[i]);

		// nack
		if(!(s == S_MST_SLAW_DATA_ACK))
			break;

		i++;
	}

end:
	/* stop and reset to not addressed slave mode */
	regs->dev->twcr = cr | (0x1 << TWCR_TWSTO) | (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

	mutex_unlock(&i2c->mtx);

	return i;
}

static int slave_read(uint8_t *buf, size_t n, void *data){
	size_t i;
	uint8_t c;
	dev_data_t *i2c;
	dt_data_t *regs;
	status_t s;


	i2c = (dev_data_t*)data;
	regs = i2c->regs;
	i = 0;

	if(i2c->regs->int_num)
		return ringbuf_read(&i2c->slave_rx_buf, buf, n);

	mutex_lock(&i2c->mtx);

	/* wait for slave being addressed */
	s = WAITINT(regs);

	if(!(s == S_SLA_SLAW_MATCH || s == S_SLA_SLAW_ARBLOST_ADDR_MATCH || s == S_SLA_BCAST_MATCH || s == S_SLA_BCAST_ARBLOST_MATCH))
		goto_errno(end, E_IO);

	DEBUG("match (%#hhx)\n", s);

	/* read bytes */
	while(i < n){
		if(i + 1 == n)	regs->dev->twcr = (regs->dev->twcr & ~(0x1 << TWCR_TWEA)) | (0x1 << TWCR_TWINT);
		else			regs->dev->twcr |= (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

		s = WAITINT(regs);
		c = regs->dev->twdr;
		DEBUG("read (%#hhx): %c (%#hhx)\n", s, c, c);

		// no data
		if(!(s == S_SLA_SLAW_DATA_ACK || s == S_SLA_SLAW_DATA_NACK || s == S_SLA_BCAST_DATA_ACK || s == S_SLA_BCAST_DATA_NACK))
			break;

		buf[i++] = c;

		// no further data, i.e. nack
		if(!(s == S_SLA_SLAW_DATA_ACK || s == S_SLA_BCAST_DATA_ACK))
			break;
	}

end:
	/* reset to not addressed slave mode */
	regs->dev->twcr |= (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

	mutex_unlock(&i2c->mtx);

	return i;
}

static int slave_write(uint8_t *buf, size_t n, void *data){
	size_t i;
	dev_data_t *i2c;
	dt_data_t *regs;
	status_t s;


	i2c = (dev_data_t*)data;
	regs = i2c->regs;
	i = 0;

	if(i2c->regs->int_num){
		n -= cmd_issue(&i2c->slave_cmd_queue, CMD_WRITE, 0, buf, n, false, i2c);
		return n;
	}

	mutex_lock(&i2c->mtx);

	/* wait for slave being addressed */
	s = WAITINT(regs);

	if(!(s == S_SLA_SLAR_ADDR_MATCH || s == S_SLA_SLAR_ARBLOST_ADDR_MATCH))
		goto_errno(end, E_IO);

	DEBUG("match (%#hhx)\n", s);

	/* write bytes */
	while(i < n){
		regs->dev->twdr = buf[i];

		if(i + 1 == n)	regs->dev->twcr = (regs->dev->twcr & ~(0x1 << TWCR_TWEA)) | (0x1 << TWCR_TWINT);
		else			regs->dev->twcr |= (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

		s = WAITINT(regs);
		DEBUG("write (%#hhx): %c (%#hhx)\n", s, buf[i], buf[i]);

		// nack
		if(!(s == S_SLA_SLAR_DATA_ACK))
			break;

		i++;
	}

end:
	/* reset to not addressed slave mode */
	regs->dev->twcr |= (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

	mutex_unlock(&i2c->mtx);

	return i;
}

static size_t cmd_issue(sigq_queue_t *queue, cmd_type_t type, uint8_t addr, void *buf, size_t n, bool trigger, dev_data_t *i2c){
	sigq_t e;
	int_cmd_t cmd;


	cmd.type = type;
	cmd.addr = addr;
	cmd.data = buf;
	cmd.len = n;

	sigq_init(&e, &cmd);

	if(sigq_enqueue(queue, &e) && trigger)
		process_int(S_NEXT_CMD, i2c, i2c->regs);

	sigq_wait(&e);

	return cmd.len;
}

static void cmd_signal(sigq_queue_t *queue, sigq_t *e){
	sigq_dequeue(queue, e);
}

static void int_hdlr(int_num_t num, void *data){
	dev_data_t *i2c;
	dt_data_t *regs;


	i2c = (dev_data_t*)data;
	regs = i2c->regs;


	process_int(STATUS(regs), i2c, regs);
}

static void process_int(status_t s, dev_data_t *i2c, dt_data_t *regs){
	int_action_t action;


	action = 0;

	mutex_lock(&i2c->mtx);

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
		regs->dev->twcr = (regs->int_num ? 0x1 : 0x0) << TWCR_TWIE
						| (((action & ACT_DO_STOP) ? 0x1 : 0x0) << TWCR_TWSTO)
						| (0x1 << TWCR_TWEA)
						| (0x1 << TWCR_TWEN)
						| (0x1 << TWCR_TWINT)
						;
	}

	mutex_unlock(&i2c->mtx);

	/* check for outstanding master operations */
	if(action)
		process_int(S_NEXT_CMD, i2c, regs);
}

static int_action_t process_master_op(status_t s, dev_data_t *i2c, dt_data_t *regs){
	static sigq_t *e = 0x0;
	uint8_t c;
	int_cmd_t *cmd;


	cmd = (e ? e->data : 0x0);

	switch(s){
	/* master start */
	case S_NEXT_CMD:
		if(e == 0x0)
			e = sigq_first(&i2c->master_cmd_queue);

		// start
		if(e){
			DEBUG("issue start\n");
			regs->dev->twcr |= (0x1 << TWCR_TWSTA) | (0x1 << TWCR_TWINT);
		}

		break;

	case S_MST_START:
	case S_MST_RESTART:
		DEBUG("start (%#hhx)\n", s);

		// sla-rw
		regs->dev->twdr = cmd->addr << TWDR_ADDR | (cmd->type << TWDR_RW);
		regs->dev->twcr = (regs->int_num ? 0x1 : 0x0) << TWCR_TWIE
	   					| (0x1 << TWCR_TWEN)
						| (0x1 << TWCR_TWINT)
						;
		break;

	case S_MST_SLAW_NACK:
	case S_MST_SLAR_NACK:
		DEBUG("nack (%#hhx)\n", s);

		// stop + start
		regs->dev->twcr |= (0x1 << TWCR_TWSTO) | (0x1 << TWCR_TWSTA) | (0x1 << TWCR_TWINT);
		break;

	case S_MST_ARBLOST:
		DEBUG("arb lost (%#hhx)\n", s);

		// start
		regs->dev->twcr |= (0x1 << TWCR_TWSTA) | (0x1 << TWCR_TWINT);
		break;

	/* master write */
	case S_MST_SLAW_DATA_ACK:
		DEBUG("write (%#hhx): %c (%#hhx)\n", s, *cmd->data, *cmd->data);

		cmd->len--;
		cmd->data++;

		// fall through
	case S_MST_SLAW_DATA_NACK:
		if(cmd->len == 0 || s == S_MST_SLAW_DATA_NACK){
			cmd_signal(&i2c->master_cmd_queue, e);
			e = 0x0;

			return ACT_DO_STOP;
		}

		// fall through
	case S_MST_SLAW_ACK:
		DEBUG("sla-w (%#hhx)\n", s);

		// write data
		regs->dev->twdr = *cmd->data;
		regs->dev->twcr |= (0x1 << TWCR_TWINT);
		break;

	/* maste read */
	case S_MST_SLAR_DATA_ACK:
	case S_MST_SLAR_DATA_NACK:
		// read data
		c = regs->dev->twdr;
		DEBUG("read (%#hhx): %c (%#hhx)\n", s, c, c);

		if(c != 0xff){
			*cmd->data = c;

			cmd->len--;
			cmd->data++;
		}

		if(cmd->len == 0 || s == S_MST_SLAR_DATA_NACK || c == 0xff){
			cmd_signal(&i2c->master_cmd_queue, e);
			e = 0x0;

			return ACT_DO_STOP;
		}

		// fall through
	case S_MST_SLAR_ACK:
		DEBUG("sla-r (%#hhx)\n", s);

		// request data
		if(cmd->len == 1)	regs->dev->twcr = (regs->dev->twcr & ~(0x1 << TWCR_TWEA)) | (0x1 << TWCR_TWINT);
		else				regs->dev->twcr |= (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

		break;

	default:
		WARN("invalid status %#hhx\n", s);
		break;
	}

	return 0;
}

static int_action_t process_slave_op(status_t s, dev_data_t *i2c, dt_data_t *regs){
	static sigq_t *e = 0x0;
	uint8_t c;
	int_cmd_t *cmd;


	cmd = (e ? e->data : 0x0);

	switch(s){
		// NOTE lost arbitration does not require any special treatment since the last master
		// 		command will be continued once the slave transmissions has beem finished

	/* slave read */
	case S_SLA_SLAW_DATA_ACK:
	case S_SLA_SLAW_DATA_NACK:
	case S_SLA_BCAST_DATA_ACK:
	case S_SLA_BCAST_DATA_NACK:
		// read data
		c = regs->dev->twdr;
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
			regs->dev->twcr = (regs->dev->twcr & ~(0x1 << TWCR_TWEA)) | (0x1 << TWCR_TWINT);
		else
			regs->dev->twcr |= (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

		break;

	/* slave write */
	case S_SLA_SLAR_DATA_ACK:
	case S_SLA_SLAR_DATA_NACK:
	case S_SLA_SLAR_DATA_LAST_ACK:
		if(cmd){
			DEBUG("write (%#hhx): %c (%#hhx)\n", s, *cmd->data, *cmd->data);

			cmd->len--;
			cmd->data++;

			if(cmd->len == 0){
				sigq_dequeue(&i2c->slave_cmd_queue, e);
				e = 0x0;
			}
		}

		if(s == S_SLA_SLAR_DATA_LAST_ACK || s == S_SLA_SLAR_DATA_NACK)
			return ACT_DO_RESET;

		// fall through
	case S_SLA_SLAR_ARBLOST_ADDR_MATCH:
	case S_SLA_SLAR_ADDR_MATCH:
		DEBUG("match (%#hhx)\n", s);

		// check for new cmd
		if(e == 0x0)
			e = sigq_first(&i2c->slave_cmd_queue);

		cmd = (e ? e->data : 0x0);

		// send data
		if(cmd)	regs->dev->twdr = *cmd->data;
		else	regs->dev->twdr = 0xff;

		if(cmd && cmd->len > 1)	regs->dev->twcr |= (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);
		else					regs->dev->twcr = (regs->dev->twcr & ~(0x1 << TWCR_TWEA)) | (0x1 << TWCR_TWINT);

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
