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
size_t avr_i2c_master_read_poll(uint8_t target_addr, uint8_t *buf, size_t n, void *data){
	size_t i;
	uint8_t c;
	avr_i2c_t *i2c;
	i2c_regs_t *regs;
	status_t s;


	i2c = (avr_i2c_t*)data;
	regs = i2c->dtd->regs;
	i = 0;

	mutex_lock(&i2c->mtx);

	/* init transfer */
	START(regs);
	s = WAITINT(regs);

	if(!(s == S_MST_START || s == S_MST_RESTART))
		goto_errno(end, E_IO);

	DEBUG("start (%#hhx)\n", s);

	SLAVE_RW(regs, target_addr, 1, false);
	s = WAITINT(regs);

	if(!(s == S_MST_SLAR_ACK))
		goto_errno(end, E_IO);

	DEBUG("sla-r (%#hhx)\n", s);

	/* read bytes */
	while(i < n){
		REQUEST_DATA(regs, (i + 1 < n));
		s = WAITINT(regs);

		c = DATA_READ(regs);
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
	/* end transfer and reset hardware */
	SET_SLAVE_MODE(regs, true, false);

	mutex_unlock(&i2c->mtx);

	return i;
}

size_t avr_i2c_master_write_poll(uint8_t target_addr, uint8_t *buf, size_t n, void *data){
	size_t i;
	avr_i2c_t *i2c;
	i2c_regs_t *regs;
	status_t s;


	i2c = (avr_i2c_t*)data;
	regs = i2c->dtd->regs;
	i = 0;

	mutex_lock(&i2c->mtx);

	/* init transfer */
	START(regs);
	s = WAITINT(regs);

	if(!(s == S_MST_START || s == S_MST_RESTART))
		goto_errno(end, E_IO);

	DEBUG("start (%#hhx)\n", s);

	SLAVE_RW(regs, target_addr, 0, false);
	s = WAITINT(regs);

	if(!(s == S_MST_SLAW_ACK))
		goto_errno(end, E_IO);

	DEBUG("sla-w (%#hhx)\n", s);

	/* write bytes */
	while(i < n){
		DATA_WRITE(regs, buf[i], true);
		s = WAITINT(regs);

		DEBUG("write (%#hhx): %c (%#hhx)\n", s, buf[i], buf[i]);

		if(!(s == S_MST_SLAW_DATA_ACK))
			break;

		i++;
	}

end:
	/* end transfer and reset hardware */
	SET_SLAVE_MODE(regs, true, false);

	mutex_unlock(&i2c->mtx);

	return i;
}

size_t avr_i2c_slave_read_poll(uint8_t *buf, size_t n, void *data){
	size_t i;
	uint8_t c;
	avr_i2c_t *i2c;
	i2c_regs_t *regs;
	status_t s;


	i2c = (avr_i2c_t*)data;
	regs = i2c->dtd->regs;
	i = 0;

	mutex_lock(&i2c->mtx);

	/* wait for slave to be addressed */
	s = WAITINT(regs);

	if(!(s == S_SLA_SLAW_MATCH || s == S_SLA_SLAW_ARBLOST_ADDR_MATCH || s == S_SLA_BCAST_MATCH || s == S_SLA_BCAST_ARBLOST_MATCH))
		goto_errno(end, E_IO);

	DEBUG("match (%#hhx)\n", s);

	/* read bytes */
	while(i < n){
		REQUEST_DATA(regs, (i + 1 < n));
		s = WAITINT(regs);

		c = DATA_READ(regs);
		DEBUG("read (%#hhx): %c (%#hhx)\n", s, c, c);

		// no data
		if(!(s == S_SLA_SLAW_DATA_ACK || s == S_SLA_SLAW_DATA_NACK || s == S_SLA_BCAST_DATA_ACK || s == S_SLA_BCAST_DATA_NACK))
			break;

		buf[i++] = c;

		// no further data
		if(!(s == S_SLA_SLAW_DATA_ACK || s == S_SLA_BCAST_DATA_ACK))
			break;
	}

end:
	/* reset hardware */
	SET_SLAVE_MODE(regs, false, false);

	mutex_unlock(&i2c->mtx);

	return i;
}

size_t avr_i2c_slave_write_poll(uint8_t *buf, size_t n, void *data){
	size_t i;
	avr_i2c_t *i2c;
	i2c_regs_t *regs;
	status_t s;


	i2c = (avr_i2c_t*)data;
	regs = i2c->dtd->regs;
	i = 0;

	mutex_lock(&i2c->mtx);

	/* wait for slave to be addressed */
	s = WAITINT(regs);

	if(!(s == S_SLA_SLAR_ADDR_MATCH || s == S_SLA_SLAR_ARBLOST_ADDR_MATCH))
		goto_errno(end, E_IO);

	DEBUG("match (%#hhx)\n", s);

	/* write bytes */
	while(i < n){
		DATA_WRITE(regs, buf[i], (i + 1 < n));
		s = WAITINT(regs);

		DEBUG("write (%#hhx): %c (%#hhx)\n", s, buf[i], buf[i]);

		if(!(s == S_SLA_SLAR_DATA_ACK))
			break;

		i++;
	}

end:
	/* reset hardware */
	SET_SLAVE_MODE(regs, false, false);

	mutex_unlock(&i2c->mtx);

	return i;
}
