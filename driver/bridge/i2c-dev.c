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


/* local/static prototypes */
static int read(i2c_t *i2c, uint8_t slave, void *buf, size_t n);
static int write(i2c_t *i2c, uint8_t slave, blob_t *bufs, size_t n);
static int rw(i2c_t *i2c, i2c_cmd_t cmd, uint8_t slave, blob_t *bufs, size_t n);

static int ack_check(i2c_t *i2c);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	bridge_cfg_t *dtd = (bridge_cfg_t*)dt_data;
	bridge_t *brdg;
	i2c_t *itf;
	i2c_ops_t ops;


	if(dtd->rx_int != 0 || dtd->tx_int != 0)
		goto_errno(err_0, E_INVAL);

	brdg = bridge_create(0x0, dtd, 0x0);

	if(brdg == 0x0)
		goto err_0;

	memset(&ops, 0x0, sizeof(i2c_ops_t));

	ops.read = read;
	ops.write = write;

	itf = i2c_create(&ops, (i2c_cfg_t*)dtd->hwcfg, brdg);

	if(itf == 0x0)
		goto err_1;

	return itf;


err_1:
	bridge_destroy(brdg);

err_0:
	return 0x0;
}

driver_probe("bridge,i2c-dev", probe);

static int read(i2c_t *i2c, uint8_t slave, void *buf, size_t n){
	return rw(i2c, I2C_CMD_READ, slave, BLOBS(BLOB(buf, n)), 1);
}

static int write(i2c_t *i2c, uint8_t slave, blob_t *bufs, size_t n){
	return rw(i2c, I2C_CMD_WRITE, slave, bufs, n);
}

static int rw(i2c_t *i2c, i2c_cmd_t cmd, uint8_t slave, blob_t *bufs, size_t n){
	i2cbrdg_hdr_t hdr;


	/* size checks */
	if(n > 255)
		return_errno(E_LIMIT);

	for(size_t i=0; i<n; i++){
		if(bufs[i].len > 255)
			return_errno(E_LIMIT);
	}

	/* write header */
	hdr.cmd = cmd;
	hdr.slave = slave;
	hdr.nbuf = n;
	hdr.len = bufs[0].len;

	if(cmd == I2C_CMD_WRITE && bufs[0].len <= CONFIG_BRIDGE_I2C_INLINE_DATA)
		memcpy(hdr.buf, bufs[0].buf, bufs[0].len);

	DEBUG("%s: slave = %u, len = %zu\n", (cmd == I2C_CMD_READ) ? "read" : "write", slave, n);

	if(i2cbrdg_write(i2c->hw, &hdr, sizeof(i2cbrdg_hdr_t)) != 0)
		goto end;

	if(ack_check(i2c) != 0)
		goto end;

	/* write payload if not already sent with the header */
	if(cmd == I2C_CMD_WRITE && (n > 1 || bufs[0].len > CONFIG_BRIDGE_I2C_INLINE_DATA)){
		for(size_t i=0; i<n; i++){
			// write payload length
			if(i2cbrdg_write(i2c->hw, (uint8_t*)(&bufs[i].len), 1) != 0)
				goto end;

			if(ack_check(i2c) != 0)
				goto end;

			// write payload
			if(i2cbrdg_write(i2c->hw, bufs[i].buf, bufs[i].len) != 0)
				goto end;
		}
	}

	/* read return value */
	if(ack_check(i2c) != 0)
		goto end;

	/* read data */
	if(cmd == I2C_CMD_READ)
		i2cbrdg_read(i2c->hw, bufs[0].buf, bufs[0].len);

end:
	DEBUG("complete: %s\n", strerror(errno));

	return -errno;
}

static int ack_check(i2c_t *i2c){
	errno_t ecode;


	BUILD_ASSERT(sizeof(errno_t) == 1);
	i2cbrdg_read(i2c->hw, &ecode, 1);

	DEBUG("ack: %s\n", strerror(ecode));

	return_errno(ecode ? ecode : errno);
}
