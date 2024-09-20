/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/driver.h>
#include <kernel/kprintf.h>
#include <driver/bridge.h>
#include <driver/i2c.h>
#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/blob.h>
#include <sys/errno.h>
#include <sys/string.h>
#include "i2c.h"


/* types */
typedef struct{
	bridge_t *brdg;

	i2c_state_t state;
	i2c_cmd_t cmd;
	uint8_t slave;

	i2c_itf_t itf;
} dev_data_t;


/* local/static prototypes */
static int configure(i2c_cfg_t *cfg, void *hw);
static i2c_state_t state(void *hw);
static void start(void *hw);
static size_t ack(size_t remaining, void *hw);
static size_t acked(size_t staged, void *hw);
static void idle(bool addressable, bool stop, void *hw);
static void connect(i2c_cmd_t cmd, uint8_t slave, void *hw);
static size_t read(uint8_t *buf, size_t n, void *hw);
static size_t write(uint8_t *buf, size_t n, bool last, void *hw);

static size_t rw(uint8_t *buf, size_t n, i2c_cmd_t cmd, uint8_t slave, bridge_t *brdg, bool last);
static int ack_check(bridge_t *brdg);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	bridge_t *dti = (bridge_t*)dt_itf;
	dev_data_t *i2c;


	if(dti->cfg->rx_int != 0 || dti->cfg->tx_int != 0)
		goto_errno(err, E_INVAL);

	i2c = kmalloc(sizeof(dev_data_t));

	if(i2c == 0x0)
		goto err;

	i2c->brdg = dti;
	i2c->state = I2C_STATE_NONE;

	i2c->itf.configure = configure;
	i2c->itf.state = state;
	i2c->itf.start = start;
	i2c->itf.ack = ack;
	i2c->itf.acked = acked;
	i2c->itf.idle = idle;
	i2c->itf.connect = connect;
	i2c->itf.read = read;
	i2c->itf.write = write;

	i2c->itf.hw = i2c;

	return &i2c->itf;


err:
	return 0x0;
}

driver_probe("bridge,i2c-dev", probe);

static int configure(i2c_cfg_t *cfg, void *hw){
	return 0;
}

static i2c_state_t state(void *hw){
	return ((dev_data_t*)hw)->state;
}

static void start(void *hw){
	((dev_data_t*)hw)->state = I2C_STATE_MST_START;
}

static size_t ack(size_t remaining, void *hw){
	dev_data_t *i2c = (dev_data_t*)hw;


	if(i2c->state == I2C_STATE_MST_RD_ACK)
		i2c->state = I2C_STATE_MST_RD_DATA_ACK;

	return remaining;
}

static size_t acked(size_t staged, void *hw){
	return staged;
}

static void idle(bool addressable, bool stop, void *hw){
	((dev_data_t*)hw)->state = I2C_STATE_NONE;
}

static void connect(i2c_cmd_t cmd, uint8_t slave, void *hw){
	dev_data_t *i2c = (dev_data_t*)hw;


	i2c->cmd = cmd;
	i2c->slave = slave;
	i2c->state = (cmd == I2C_READ) ? I2C_STATE_MST_RD_ACK : I2C_STATE_MST_WR_ACK;
}

static size_t read(uint8_t *buf, size_t n, void *hw){
	dev_data_t *i2c = (dev_data_t*)hw;


	n = rw(buf, n, i2c->cmd, i2c->slave, i2c->brdg, true);
	i2c->state = (n == 0) ? I2C_STATE_ERROR : I2C_STATE_MST_RD_DATA_ACK;

	return n;
}

static size_t write(uint8_t *buf, size_t n, bool last, void *hw){
	dev_data_t *i2c = (dev_data_t*)hw;


	n = rw(buf, n, i2c->cmd, i2c->slave, i2c->brdg, last);
	i2c->state = (n == 0) ? I2C_STATE_ERROR : I2C_STATE_MST_WR_DATA_ACK;

	return n;
}

static size_t rw(uint8_t *buf, size_t n, i2c_cmd_t cmd, uint8_t slave, bridge_t *brdg, bool last){
	i2cbrdg_hdr_t hdr;


	/* size checks */
	if(n > 255)
		goto_errno(err, E_LIMIT);

	/* write header */
	hdr.cmd = cmd;
	hdr.slave = slave;
	hdr.last = last;
	hdr.len = n;

	if((cmd == I2C_WRITE) && n <= CONFIG_BRIDGE_I2C_INLINE_DATA)
		memcpy(hdr.buf, buf, n);

	DEBUG("%s: slave=%u, len=%zu, last=%u\n", (cmd == I2C_READ) ? "read" : "write", slave, n, last);

	if(i2cbrdg_write(brdg, &hdr, sizeof(i2cbrdg_hdr_t)) != 0)
		goto err;

	if(ack_check(brdg) != 0)
		goto err;

	/* write payload if not already sent with the header */
	if((cmd == I2C_WRITE) && (n > CONFIG_BRIDGE_I2C_INLINE_DATA)){
		if(i2cbrdg_write(brdg, buf, n) != 0)
			goto err;
	}

	/* read return value */
	if(last && ack_check(brdg) != 0)
		goto err;

	/* read data */
	if(cmd == I2C_READ){
		if(i2cbrdg_read(brdg, buf, n) != 0)
			goto err;
	}

	return n;


err:
	DEBUG("error: %s\n", strerror(errno));

	return 0;
}

static int ack_check(bridge_t *brdg){
	errno_t errnum;


	STATIC_ASSERT(sizeof(errno_t) == 1);
	i2cbrdg_read(brdg, &errnum, 1);

	DEBUG("ack: %s\n", strerror(errnum));

	return_errno(errnum ? errnum : errno);
}
