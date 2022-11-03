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
#include <sys/compiler.h>
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
	dev_state_t state;

	i2cbrdg_hdr_t hdr;
	void *buf;

	uint8_t nbuf;
	blob_t *bufs;
} dev_data_t;


/* local/static prototypes */
static void rx_hdlr(int_num_t num, void *payload);

static int rx(dev_data_t *i2c);
static int rx_header(dev_data_t *i2c, i2cbrdg_hdr_t *hdr);
static int rx_payload_len(dev_data_t *i2c, i2cbrdg_hdr_t *hdr);
static int rx_payload(dev_data_t *i2c, i2cbrdg_hdr_t *hdr);
static void exec(dev_data_t *i2c, i2cbrdg_hdr_t *hdr);

static int ack(dev_data_t *i2c, errno_t ecode);
static void reset(dev_data_t *i2c);


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	bridge_cfg_t *dtd = (bridge_cfg_t*)dt_data;
	i2c_t *dti = (i2c_t*)dt_itf;
	dev_data_t *i2c;
	bridge_t *brdg;


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

	brdg = bridge_create(0x0, dtd, dti);

	if(brdg == 0)
		goto err_0;

	i2c = kcalloc(1, sizeof(dev_data_t));

	if(i2c == 0x0)
		goto err_1;

	i2c->brdg = brdg;

	reset(i2c);

	if(int_register(dtd->rx_int, rx_hdlr, i2c) != 0)
		goto err_2;

	return 0x0;


err_2:
	kfree(i2c);

err_1:
	bridge_destroy(brdg);

err_0:
	return 0x0;
}

driver_probe("bridge,i2c-itf", probe);

static void rx_hdlr(int_num_t num, void *payload){
	dev_data_t *i2c = (dev_data_t*)payload;


	i2c->state = rx(i2c);

	if(i2c->state == DS_ERROR)
		return reset(i2c);

	if(i2c->state != DS_EXEC)
		return;

	exec(i2c, &i2c->hdr);
	reset(i2c);
}

static int rx(dev_data_t *i2c){
	switch(i2c->state){
	case DS_HEADER:			return rx_header(i2c, &i2c->hdr);
	case DS_PAYLOAD_LEN:	return rx_payload_len(i2c, &i2c->hdr);
	case DS_PAYLOAD:		return rx_payload(i2c, &i2c->hdr);
	default:				return DS_ERROR;
	}
}

static int rx_header(dev_data_t *i2c, i2cbrdg_hdr_t *hdr){
	/* read header */
	if(i2cbrdg_read(i2c->brdg, hdr, sizeof(i2cbrdg_hdr_t)) != 0)
		return DS_ERROR;

	DEBUG("%s: slave = %u, nbuf = %zu, len = %zu\n",
		(hdr->cmd == I2C_CMD_READ) ? "read" : "write",
		hdr->slave,
		hdr->nbuf,
		hdr->len
	);

	/* allocate buffers */
	if(hdr->nbuf > 1 || hdr->len > CONFIG_BRIDGE_I2C_INLINE_DATA){
		i2c->bufs = kmalloc(hdr->nbuf * sizeof(blob_t));

		if(hdr->cmd == I2C_CMD_READ && i2c->bufs)
			i2c->bufs[0].buf = kmalloc(hdr->len);

		i2c->buf = i2c->bufs[0].buf;
	}

	/* send ack */
	if(ack(i2c, errno) != 0)
		return DS_ERROR;

	if(hdr->cmd == I2C_CMD_WRITE && (hdr->nbuf > 1 || hdr->len > CONFIG_BRIDGE_I2C_INLINE_DATA))
		return DS_PAYLOAD_LEN;

	return DS_EXEC;
}

static int rx_payload_len(dev_data_t *i2c, i2cbrdg_hdr_t *hdr){
	uint8_t len;


	if(i2cbrdg_read(i2c->brdg, &len, 1) != 0)
		return DS_ERROR;

	i2c->bufs[i2c->nbuf].buf = kmalloc(len);
	i2c->bufs[i2c->nbuf].len = len;
	i2c->buf = i2c->bufs[0].buf;

	if(ack(i2c, errno) != 0)
		return DS_ERROR;

	return DS_PAYLOAD;
}

static int rx_payload(dev_data_t *i2c, i2cbrdg_hdr_t *hdr){
	if(i2cbrdg_read(i2c->brdg, i2c->bufs[i2c->nbuf].buf, i2c->bufs[i2c->nbuf].len) != 0)
		return DS_ERROR;

	i2c->nbuf++;

	return (i2c->nbuf == hdr->nbuf) ? DS_EXEC : DS_PAYLOAD_LEN;
}

static void exec(dev_data_t *i2c, i2cbrdg_hdr_t *hdr){
	int r;


	/* perform i2c operations */
	DEBUG("issue i2c command\n");

	if(hdr->cmd == I2C_CMD_WRITE){
		r = (hdr->nbuf == 1)
		  ? i2c_write(i2c->brdg->hw, hdr->slave, i2c->buf, hdr->len)
		  : i2c_write_n(i2c->brdg->hw, hdr->slave, i2c->bufs, i2c->nbuf)
		;
	}
	else
		r = i2c_read(i2c->brdg->hw, hdr->slave, i2c->buf, hdr->len);

	if(ack(i2c, ((r == 0) ? 0 : (errno ? errno : E_AGAIN))) != 0)
		return;

	if(hdr->cmd == I2C_CMD_READ)
		(void)i2cbrdg_write(i2c->brdg, i2c->buf, hdr->len);
}

static int ack(dev_data_t *i2c, errno_t ecode){
	DEBUG("ack: %d\n", ecode);

	STATIC_ASSERT(sizeof(errno_t) == 1);
	(void)i2cbrdg_write(i2c->brdg, &ecode, 1);

	return ecode;
}

static void reset(dev_data_t *i2c){
	for(uint8_t i=0; i<i2c->nbuf; i++)
		kfree(i2c->bufs[i].buf);

	kfree(i2c->bufs);

	i2c->state = DS_HEADER;
	i2c->nbuf = 0;
	i2c->bufs = 0x0;
	i2c->buf = i2c->hdr.buf;
}
