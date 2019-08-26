/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/kprintf.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mutex.h>
#include "itf.h"



/* global functions */
int avr_i2c_master_read_poll(uint8_t target_addr, uint8_t *buf, size_t n, void *data){
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
	/* stop and reset to not addressed slave mode */
	regs->dev->twcr = cr | (0x1 << TWCR_TWSTO) | (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

	mutex_unlock(&i2c->mtx);

	return i;
}

int avr_i2c_master_write_poll(uint8_t target_addr, uint8_t *buf, size_t n, void *data){
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
	/* stop and reset to not addressed slave mode */
	regs->dev->twcr = cr | (0x1 << TWCR_TWSTO) | (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

	mutex_unlock(&i2c->mtx);

	return i;
}

int avr_i2c_slave_read_poll(uint8_t *buf, size_t n, void *data){
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
	/* reset to not addressed slave mode */
	regs->dev->twcr |= (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

	mutex_unlock(&i2c->mtx);

	return i;
}

int avr_i2c_slave_write_poll(uint8_t *buf, size_t n, void *data){
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
	/* reset to not addressed slave mode */
	regs->dev->twcr |= (0x1 << TWCR_TWEA) | (0x1 << TWCR_TWINT);

	mutex_unlock(&i2c->mtx);

	return i;
}
