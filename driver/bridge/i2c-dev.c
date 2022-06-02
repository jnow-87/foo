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
#include <sys/errno.h>
#include <sys/string.h>
#include "i2c.h"


/* local/static prototypes */
static size_t read(i2c_t *i2c, uint8_t slave, void *buf, size_t n);
static size_t write(i2c_t *i2c, uint8_t slave, void *buf, size_t n);
static size_t rw(i2c_t *i2c, i2c_cmd_t cmd, uint8_t slave, void *buf, size_t n);

static int ack_check(i2c_t *i2c);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	bridge_t *brdg;
	bridge_cfg_t *dtd;
	i2c_t *itf;
	i2c_ops_t ops;


	dtd = (bridge_cfg_t*)dt_data;

	if(dtd->rx_int != 0 || dtd->tx_int != 0)
		goto_errno(err_0, E_INVAL);

	brdg = bridge_create(dtd, 0x0, 0x0);

	if(brdg == 0x0)
		goto err_0;

	memset(&ops, 0x0, sizeof(i2c_ops_t));

	ops.read = read;
	ops.write = write;

	itf = i2c_create(&ops, (i2c_cfg_t*)dtd->hw_cfg, brdg);

	if(itf == 0x0)
		goto err_1;

	return itf;


err_1:
	bridge_destroy(brdg);

err_0:
	return 0x0;
}

driver_probe("bridge,i2c-dev", probe);

static size_t read(i2c_t *i2c, uint8_t slave, void *buf, size_t n){
	return rw(i2c, I2C_CMD_READ, slave, buf, n);
}

static size_t write(i2c_t *i2c, uint8_t slave, void *buf, size_t n){
	return rw(i2c, I2C_CMD_WRITE, slave, buf, n);
}

static size_t rw(i2c_t *i2c, i2c_cmd_t cmd, uint8_t slave, void *buf, size_t n){
	i2cbrdg_hdr_t hdr;


	hdr.cmd = cmd;
	hdr.slave = slave;
	hdr.len = n;

	if(cmd == I2C_CMD_WRITE && n <= CONFIG_BRIDGE_I2C_INLINE_DATA)
		memcpy(hdr.data, buf, n);

	DEBUG("%s: slave = %u, len = %zu\n", (cmd == I2C_CMD_READ) ? "read" : "write", slave, n);

	/* write header */
	if(i2cbrdg_write(i2c->hw, &hdr, sizeof(i2cbrdg_hdr_t)) != 0)
		goto end;

	if(ack_check(i2c) != 0)
		goto end;

	/* write payload if not already sent with the header */
	if(cmd == I2C_CMD_WRITE && n > CONFIG_BRIDGE_I2C_INLINE_DATA){
		if(i2cbrdg_write(i2c->hw, buf, n) != 0)
			goto end;
	}

	/* read return value */
	if(ack_check(i2c) != 0)
		goto end;

	/* read data */
	if(cmd == I2C_CMD_READ)
		i2cbrdg_read(i2c->hw, buf, n);

end:
	DEBUG("complete: %s\n", strerror(errno));

	return errno ? 0 : n;
}

static int ack_check(i2c_t *i2c){
	errno_t ecode;


	BUILD_ASSERT(sizeof(errno_t) == 1);
	i2cbrdg_read(i2c->hw, &ecode, 1);

	DEBUG("ack: %s\n", strerror(ecode));

	return_errno(ecode ? ecode : errno);
}
