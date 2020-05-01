/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/kprintf.h>
#include <driver/i2c.h>
#include <sys/types.h>
#include <sys/errno.h>


/* global functions */
size_t i2c_master_read(uint8_t remote, uint8_t *buf, size_t n, i2c_primitives_t *prim){
	size_t i;
	uint8_t c;
	i2c_state_t s;


	i = 0;

	/* init transfer */
	prim->start(prim->data);
	s = prim->state(true, prim->data);

	DEBUG("start (%#hhx)\n", s);

	if(!(s == I2C_STATE_MST_START || s == I2C_STATE_MST_RESTART))
		goto_errno(end, E_IO);

	prim->slave_read_write(remote, true, false, prim->data);
	s = prim->state(true, prim->data);

	DEBUG("sla-r (%#hhx)\n", s);

	if(!(s == I2C_STATE_MST_SLAR_ACK))
		goto_errno(end, E_NOCONN);

	/* read bytes */
	while(i < n){
		prim->byte_request((i + 1 < n), prim->data);
		s = prim->state(true, prim->data);

		c = prim->byte_read(prim->data);
		DEBUG("read (%#hhx): %c (%#hhx)\n", s, c, c);

		// no data or invalid data (0xff)
		if(!(s == I2C_STATE_MST_SLAR_DATA_ACK || s == I2C_STATE_MST_SLAR_DATA_NACK) || c == 0xff)
			break;

		buf[i++] = c;

		// no further data, i.e. nack
		if(!(s == I2C_STATE_MST_SLAR_DATA_ACK))
			break;
	}

end:
	/* end transfer and reset hardware */
	prim->slave_mode_set(true, false, prim->data);

	return i;
}

size_t i2c_master_write(uint8_t remote, uint8_t *buf, size_t n, i2c_primitives_t *prim){
	size_t i;
	i2c_state_t s;


	i = 0;

	/* init transfer */
	prim->start(prim->data);
	s = prim->state(true, prim->data);

	DEBUG("start (%#hhx)\n", s);

	if(!(s == I2C_STATE_MST_START || s == I2C_STATE_MST_RESTART))
		goto_errno(end, E_IO);

	prim->slave_read_write(remote, false, false, prim->data);
	s = prim->state(true, prim->data);

	DEBUG("sla-w (%#hhx)\n", s);

	if(!(s == I2C_STATE_MST_SLAW_ACK))
		goto_errno(end, E_NOCONN);

	/* write bytes */
	while(i < n){
		prim->byte_write(buf[i], true, prim->data);
		s = prim->state(true, prim->data);

		DEBUG("write (%#hhx): %c (%#hhx)\n", s, buf[i], buf[i]);

		if(!(s == I2C_STATE_MST_SLAW_DATA_ACK))
			break;

		i++;
	}

end:
	/* end transfer and reset hardware */
	prim->slave_mode_set(true, false, prim->data);

	return i;
}

size_t i2c_slave_read(uint8_t *buf, size_t n, i2c_primitives_t *prim){
	size_t i;
	uint8_t c;
	i2c_state_t s;


	i = 0;

	/* wait for slave to be addressed */
	s = prim->state(true, prim->data);

	if(!(s == I2C_STATE_SLA_SLAW_MATCH
	  || s == I2C_STATE_SLA_SLAW_ARBLOST_ADDR_MATCH
	  || s == I2C_STATE_SLA_BCAST_MATCH
	  || s == I2C_STATE_SLA_BCAST_ARBLOST_MATCH)
	)
		goto_errno(end, E_IO);

	DEBUG("match (%#hhx)\n", s);

	/* read bytes */
	while(i < n){
		prim->byte_request((i + 1 < n), prim->data);
		s = prim->state(true, prim->data);

		c = prim->byte_read(prim->data);
		DEBUG("read (%#hhx): %c (%#hhx)\n", s, c, c);

		// no data
		if(!(s == I2C_STATE_SLA_SLAW_DATA_ACK
		  || s == I2C_STATE_SLA_SLAW_DATA_NACK
		  || s == I2C_STATE_SLA_BCAST_DATA_ACK
		  || s == I2C_STATE_SLA_BCAST_DATA_NACK)
		)
			break;

		buf[i++] = c;

		// no further data
		if(!(s == I2C_STATE_SLA_SLAW_DATA_ACK || s == I2C_STATE_SLA_BCAST_DATA_ACK))
			break;
	}

end:
	/* reset hardware */
	prim->slave_mode_set(false, false, prim->data);

	return i;
}

size_t i2c_slave_write(uint8_t *buf, size_t n, i2c_primitives_t *prim){
	size_t i;
	i2c_state_t s;


	i = 0;

	/* wait for slave to be addressed */
	s = prim->state(true, prim->data);

	if(!(s == I2C_STATE_SLA_SLAR_ADDR_MATCH || s == I2C_STATE_SLA_SLAR_ARBLOST_ADDR_MATCH))
		goto_errno(end, E_IO);

	DEBUG("match (%#hhx)\n", s);

	/* write bytes */
	while(i < n){
		prim->byte_write(buf[i], (i + 1 < n), prim->data);
		s = prim->state(true, prim->data);

		DEBUG("write (%#hhx): %c (%#hhx)\n", s, buf[i], buf[i]);

		if(!(s == I2C_STATE_SLA_SLAR_DATA_ACK))
			break;

		i++;
	}

end:
	/* reset hardware */
	prim->slave_mode_set(false, false, prim->data);

	return i;
}
