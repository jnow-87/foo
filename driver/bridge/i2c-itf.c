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
#include <sys/blob.h>
#include <sys/errno.h>
#include "i2c.h"


/* types */
typedef enum{
	DS_HEADER = 0,
	DS_PAYLOAD_LEN,
	DS_PAYLOAD,
	DS_EXEC,
	DS_ERROR,
} dev_state_t;

typedef struct{
	bridge_t *brdg;
	i2c_t *i2c;

	dev_state_t state;

	i2cbrdg_hdr_t hdr;

	uint8_t nbuf;
	blob_t *bufs;

	void *buf;
} dev_data_t;


/* local/static prototypes */
static void rx_hdlr(int_num_t num, void *data);

static int rx(dev_data_t *dev);
static int rx_header(dev_data_t *dev, i2cbrdg_hdr_t *hdr);
static int rx_payload_len(dev_data_t *dev, i2cbrdg_hdr_t *hdr);
static int rx_payload(dev_data_t *dev, i2cbrdg_hdr_t *hdr);
static void exec(dev_data_t *dev, i2cbrdg_hdr_t *hdr);

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
	dev_data_t *dev;


	dev = (dev_data_t*)data;

	dev->state = rx(dev);

	if(dev->state == DS_ERROR)
		return reset(dev);

	if(dev->state != DS_EXEC)
		return;

	exec(dev, &dev->hdr);
	reset(dev);
}

static int rx(dev_data_t *dev){
	switch(dev->state){
	case DS_HEADER:			return rx_header(dev, &dev->hdr);
	case DS_PAYLOAD_LEN:	return rx_payload_len(dev, &dev->hdr);
	case DS_PAYLOAD:		return rx_payload(dev, &dev->hdr);
	default:				return DS_ERROR;
	}
}

static int rx_header(dev_data_t *dev, i2cbrdg_hdr_t *hdr){
	/* read header */
	if(i2cbrdg_read(dev->brdg, hdr, sizeof(i2cbrdg_hdr_t)) != 0)
		return DS_ERROR;

	DEBUG("%s: slave = %u, nbuf = %zu, len = %zu\n",
		(hdr->cmd == I2C_CMD_READ) ? "read" : "write",
		hdr->slave,
		hdr->nbuf,
		hdr->len
	);

	/* allocate buffers */
	if(hdr->nbuf > 1 || hdr->len > CONFIG_BRIDGE_I2C_INLINE_DATA){
		dev->bufs = kmalloc(hdr->nbuf * sizeof(blob_t));

		if(hdr->cmd == I2C_CMD_READ && dev->bufs)
			dev->bufs[0].data = kmalloc(hdr->len);

		dev->buf = dev->bufs[0].data;
	}

	/* send ack */
	if(ack(dev, errno) != 0)
		return DS_ERROR;

	if(hdr->cmd == I2C_CMD_WRITE && (hdr->nbuf > 1 || hdr->len > CONFIG_BRIDGE_I2C_INLINE_DATA))
		return DS_PAYLOAD_LEN;

	return DS_EXEC;
}

static int rx_payload_len(dev_data_t *dev, i2cbrdg_hdr_t *hdr){
	uint8_t len;


	if(i2cbrdg_read(dev->brdg, &len, 1) != 0)
		return DS_ERROR;

	dev->bufs[dev->nbuf].data = kmalloc(len);
	dev->bufs[dev->nbuf].len = len;
	dev->buf = dev->bufs[0].data;

	if(ack(dev, errno) != 0)
		return DS_ERROR;

	return DS_PAYLOAD;
}

static int rx_payload(dev_data_t *dev, i2cbrdg_hdr_t *hdr){
	if(i2cbrdg_read(dev->brdg, dev->bufs[dev->nbuf].data, dev->bufs[dev->nbuf].len) != 0)
		return DS_ERROR;

	dev->nbuf++;

	return (dev->nbuf == hdr->nbuf) ? DS_EXEC : DS_PAYLOAD_LEN;
}

static void exec(dev_data_t *dev, i2cbrdg_hdr_t *hdr){
	int r;


	/* perform i2c operations */
	DEBUG("issue i2c command\n");

	if(hdr->cmd == I2C_CMD_WRITE){
		r = (hdr->nbuf == 1) ?
			i2c_write(dev->brdg->hw, hdr->slave, dev->buf, hdr->len) :
			i2c_write_n(dev->brdg->hw, hdr->slave, dev->bufs, dev->nbuf)
		;
	}
	else
		r = i2c_read(dev->brdg->hw, hdr->slave, dev->buf, hdr->len);

	if(ack(dev, ((r == 0) ? E_OK : (errno ? errno : E_AGAIN))) != 0)
		return;

	if(hdr->cmd == I2C_CMD_READ)
		(void)i2cbrdg_write(dev->brdg, dev->buf, hdr->len);
}

static int ack(dev_data_t *dev, errno_t ecode){
	DEBUG("ack: %d\n", ecode);

	BUILD_ASSERT(sizeof(errno_t) == 1);
	(void)i2cbrdg_write(dev->brdg, &ecode, 1);

	return ecode;
}

static void reset(dev_data_t *dev){
	uint8_t i;


	for(i=0; i<dev->nbuf; i++)
		kfree(dev->bufs[i].data);

	kfree(dev->bufs);

	dev->state = DS_HEADER;
	dev->nbuf = 0;
	dev->bufs = 0x0;
	dev->buf = dev->hdr.data;
}
