/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/interrupt.h>
#include <arch/avr/register.h>
#include <kernel/driver.h>
#include <kernel/kprintf.h>
#include <driver/i2c.h>
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

typedef struct{
	i2c_itf_t itf;
	dt_data_t *regs;

	mutex_t mtx;
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
} status_t;


/* local/static prototypes */
static int configure(i2c_cfg_t *cfg, void *data);
static int master_read(uint8_t target_addr, uint8_t *buf, size_t n, void *data);
static int master_write(uint8_t target_addr, uint8_t *buf, size_t n, void *data);
static int slave_read(uint8_t *buf, size_t n, void *data);
static int slave_write(uint8_t *buf, size_t n, void *data);

static void int_hdlr(int_num_t num, void *data);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dev_data_t *i2c;
	dt_data_t *regs;


	regs = (dt_data_t*)dt_data;

	/* allocate interface */
	i2c = kmalloc(sizeof(dev_data_t));

	if(i2c == 0x0)
		goto err_0;

	i2c->itf.configure = configure;
	i2c->itf.master_read = master_read;
	i2c->itf.master_write = master_write;
	i2c->itf.slave_read = slave_read;
	i2c->itf.slave_write = slave_write;
	i2c->itf.data = i2c;
	i2c->regs = regs;

	mutex_init(&i2c->mtx, MTX_NONE);

	/* register interrupt */
	if(regs->int_num && int_register(regs->int_num, int_hdlr, i2c) != 0)
		goto err_1;

	return &i2c->itf;


err_1:
	kfree(i2c);

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
	/* stop and reset to unaddressed slave mode */
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
	/* stop and reset to unaddressed slave mode */
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
	/* reset to unaddressed slave mode */
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
	/* reset to unaddressed slave mode */
	regs->dev->twcr |= (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

	mutex_unlock(&i2c->mtx);

	return i;
}

static void int_hdlr(int_num_t num, void *data){
}
