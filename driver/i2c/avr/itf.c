/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/interrupt.h>
#include <kernel/memory.h>
#include <kernel/driver.h>
#include <kernel/inttask.h>
#include <driver/i2c.h>
#include <sys/ringbuf.h>
#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mutex.h>
#include "itf.h"


/* macros */
#if !defined(CONFIG_I2C_AVR_INTERRUPT) && !defined(CONFIG_I2C_AVR_POLLING)
GCC_ERROR("either interrupt or polling support has to be enabled for the avr i2c driver")
#endif


/* local/static prototypes */
static int configure(i2c_cfg_t *cfg, void *data);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	void *buf;
	dt_data_t *dtd;
	avr_i2c_t *i2c;


	dtd = (dt_data_t*)dt_data;

	/* allocate rx buffer */
	buf = 0x0;

#ifdef CONFIG_I2C_AVR_INTERRUPT
	if(dtd->int_num){
		buf = kmalloc(CONFIG_I2C_RXBUF_SIZE);

		if(buf == 0x0)
			goto err_0;
	}
#endif // CONFIG_I2C_AVR_INTERRUPT

	/* allocate interface */
	i2c = kmalloc(sizeof(avr_i2c_t));

	if(i2c == 0x0)
		goto err_1;

	i2c->hw.configure = configure;

	if(dtd->int_num){
#ifdef CONFIG_I2C_AVR_INTERRUPT
		i2c->hw.master_read = avr_i2c_master_read_int;
		i2c->hw.master_write = avr_i2c_master_write_int;
		i2c->hw.slave_read = avr_i2c_slave_read_int;
		i2c->hw.slave_write = avr_i2c_slave_write_int;
#else
		goto_errno(err_1, E_NOSUP);
#endif // CONFIG_I2C_AVR_INTERRUPT
	}
	else{
#ifdef CONFIG_I2C_AVR_POLLING
		i2c->hw.master_read = avr_i2c_master_read_poll;
		i2c->hw.master_write = avr_i2c_master_write_poll;
		i2c->hw.slave_read = avr_i2c_slave_read_poll;
		i2c->hw.slave_write = avr_i2c_slave_write_poll;
#else
		goto_errno(err_0, E_NOSUP);
#endif // CONFIG_I2C_AVR_POLLING
	}

	i2c->hw.data = i2c;
	i2c->dtd = dtd;

	mutex_init(&i2c->mtx, MTX_NONE);
	itask_queue_init(&i2c->master_cmd_queue);
	itask_queue_init(&i2c->slave_cmd_queue);
	ringbuf_init(&i2c->slave_rx_buf, buf, CONFIG_I2C_RXBUF_SIZE);

	/* register interrupt */
#ifdef CONFIG_I2C_AVR_INTERRUPT
	if(dtd->int_num && int_register(dtd->int_num, avr_i2c_int_hdlr, i2c) != 0)
		goto err_2;
#endif // CONFIG_I2C_AVR_INTERRUPT

	return &i2c->hw;


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
	dt_data_t *dtd;
	i2c_regs_t *regs;
	avr_i2c_t *i2c;


	i2c = (avr_i2c_t*)data;
	dtd = i2c->dtd;
	regs = dtd->regs;

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
	*dtd->prr |= dtd->prr_en;

	/* re-enable twi */
	*dtd->prr &= ~dtd->prr_en;

	regs->twsr = 0x0;
	regs->twbr = brate;
	regs->twamr = 0x0;
	regs->twar = cfg->host_addr << 0x1 | ((cfg->bcast_en ? 0x1 : 0x0) << TWAR_TWGCE);
	regs->twcr = (0x1 << TWCR_TWEN)
					| (0x1 << TWCR_TWEA)
					| ((dtd->int_num ? 0x1 : 0x0) << TWCR_TWIE)
					;

	mutex_unlock(&i2c->mtx);

	return E_OK;
}
