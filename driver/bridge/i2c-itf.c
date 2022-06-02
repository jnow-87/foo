/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/interrupt.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>
#include <driver/bridge.h>
#include <driver/i2c.h>
#include <sys/types.h>
#include <sys/errno.h>
#include "i2c.h"


/* types */
typedef struct{
	bridge_t *brdg;
	i2c_t *i2c;

	i2cbrdg_hdr_t hdr;
	void *rx_buf;
} dev_data_t;


/* local/static prototypes */
static void rx_hdlr(int_num_t num, void *data);

static int ack(dev_data_t *dev, errno_t ecode);
static void reset(dev_data_t *dev);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	dev_data_t *dev;
	bridge_t *brdg;
	bridge_cfg_t *dtd;
	i2c_t *dti;


	dtd = (bridge_cfg_t*)dt_data;
	dti = (i2c_t*)dt_itf;

	// disable bridge tx interrupt to spare the tx handler function
	dtd->tx_int = 0;

	// Ensure the rx interrupt is set for the bridge and no interrupts are set 
	// for the underlying i2c device. There is no user interaction with the
	// bridge, thus its functions have to be triggered through interrupts. As a
	// consequence the i2c device must not be used in interrupt mode, since the
	// i2c functions use itasks, which rely on sched_yield() that must not be
	// called from within interrupt handlers.
	if(dtd->rx_int == 0 || dti->cfg->int_num != 0)
		goto_errno(err_0, E_INVAL);

	brdg = bridge_create(dtd, 0x0, dti);

	if(brdg == 0)
		goto err_0;

	dev = kcalloc(1, sizeof(dev_data_t));

	if(dev == 0x0)
		goto err_1;

	dev->brdg = brdg;
	dev->i2c = dti;

	reset(dev);

	if(int_register(dtd->rx_int, rx_hdlr, dev) != 0)
		goto err_2;

	return 0x0;


err_2:
	kfree(dev);

err_1:
	bridge_destroy(brdg);

err_0:
	return 0x0;
}

driver_probe("bridge,i2c-itf", probe);

static void rx_hdlr(int_num_t num, void *data){
	size_t r;
	dev_data_t *dev;
	i2cbrdg_hdr_t *hdr;


	dev = (dev_data_t*)data;
	hdr = &dev->hdr;

	if(hdr->len == 0){
		/* read header */
		if(i2cbrdg_read(dev->brdg, hdr, sizeof(i2cbrdg_hdr_t)) != 0)
			return reset(dev);

		DEBUG("%s: slave = %u, len = %zu\n", (hdr->cmd == I2C_CMD_READ) ? "read" : "write", hdr->slave, hdr->len);

		/* prepare rx buffer */
		if(hdr->len > CONFIG_BRIDGE_I2C_INLINE_DATA)
			dev->rx_buf = kmalloc(hdr->len);

		/* send ack */
		if(ack(dev, errno) != 0)
			return;

		if(hdr->cmd == I2C_CMD_WRITE && hdr->len > CONFIG_BRIDGE_I2C_INLINE_DATA)
			return;
	}
	else{
		/* read payload */
		if(i2cbrdg_read(dev->brdg, dev->rx_buf, hdr->len) != 0)
			return reset(dev);
	}

	/* perform i2c operations */
	DEBUG("issue i2c command\n");

	r = ((hdr->cmd == I2C_CMD_READ) ?
		i2c_read(dev->brdg->hw, hdr->slave, dev->rx_buf, hdr->len) :
		i2c_write(dev->brdg->hw, hdr->slave, dev->rx_buf, hdr->len)
	);

	if(ack(dev, (r == hdr->len) ? E_OK : (errno ? errno : E_AGAIN)) != 0)
		return;

	if(dev->hdr.cmd == I2C_CMD_READ)
		(void)i2cbrdg_write(dev->brdg, dev->rx_buf, dev->hdr.len);

	/* cleanup */
	reset(dev);
}

static int ack(dev_data_t *dev, errno_t ecode){
	if(ecode != E_OK)
		reset(dev);

	DEBUG("ack: %d\n", ecode);

	BUILD_ASSERT(sizeof(errno_t) == 1);
	(void)i2cbrdg_write(dev->brdg, &ecode, 1);

	return ecode;
}

static void reset(dev_data_t *dev){
	if(dev->rx_buf != dev->hdr.data)
		kfree(dev->rx_buf);

	dev->hdr.len = 0;
	dev->rx_buf = dev->hdr.data;
}
