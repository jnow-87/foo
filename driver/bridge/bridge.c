/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/memory.h>
#include <kernel/interrupt.h>
#include <kernel/kprintf.h>
#include <driver/bridge.h>
#include <sys/types.h>
#include <sys/mutex.h>
#include <sys/list.h>
#include <sys/math.h>
#include <sys/string.h>


/* macros */
#define CHUNKSIZE(cs)	(0x1 << (cs))

#define CHUNKS(n, cs)({ \
	uint8_t x; \
	\
	\
	x = (n) / (cs); \
	x += ((x * (cs)) == n) ? 0 : 1; \
	\
	x; \
})


/* local/static prototypes */
static int read(bridge_t *brdg, uint8_t *data, uint8_t n, uint8_t *expect);
static int write(bridge_t *brdg, uint8_t const *data, uint8_t n);

static int ack(bridge_t *brdg, uint8_t byte);
static int nack(bridge_t *brdg, uint8_t byte);

static uint8_t checksum(uint8_t const *data, size_t n);

static void rx_hdlr(int_num_t num, void *brdg);
static void tx_hdlr(int_num_t num, void *brdg);


/* static variables */
static bridge_t *bridge_lst = 0x0;
static mutex_t bridge_mtx = MUTEX_INITIALISER();


/**
 * TODO
 * 	- handle errors in brdg->read/write
 * 	- apply timeouts
 */

/* global functions */
bridge_t *bridge_create(bridge_cfg_t *cfg, bridge_ops_t *ops, void *hw_itf){
	bridge_t *brdg;


	/* allocate bridge and terminal */
	brdg = kcalloc(1, sizeof(bridge_t));

	if(brdg == 0x0)
		goto err_0;

	/* register interrupts */
	if(cfg->rx_int && int_register(cfg->rx_int, rx_hdlr, brdg) != 0)
		goto err_1;

	if(cfg->rx_int && int_register(cfg->tx_int, tx_hdlr, brdg) != 0)
		goto err_1;

	/* init bridge and configure hardware */
	brdg->cfg = cfg;
	brdg->hw = hw_itf;
	brdg->ops = *ops;

	list_for_each(bridge_lst, brdg->peer){
		if(brdg->peer->cfg->id == cfg->id)
			break;
	}

	if(brdg->peer){
		if(brdg->peer->peer != 0x0){
			FATAL("bridge id %hhu used more than twice\n", cfg->id);
			goto_errno(err_1, E_INUSE);
		}

		brdg->peer->peer = brdg;
	}

	list_add_tail_safe(bridge_lst, brdg, &bridge_mtx);

	return brdg;


err_1:
	bridge_destroy(brdg);

err_0:
	return 0x0;
}

void bridge_destroy(bridge_t *brdg){
	bridge_cfg_t *cfg;


	cfg = brdg->cfg;

	if(cfg->rx_int)
		int_release(cfg->rx_int);

	if(cfg->tx_int)
		int_release(cfg->tx_int);

	kfree(brdg);
}

int16_t bridge_read(bridge_t *brdg, void *data, uint8_t n){
	uint8_t i,
			b,
			chunks,
			chunksize,
			csum;


	// read control byte
	if(read(brdg, &b, 1, (uint8_t []){ ((brdg->seq_num << 3) | brdg->cfg->chunksize_e) }) != 0)
		goto err;

	DEBUG("control byte %#hhx\n", b);

	// read data length
	if(read(brdg, &b, 1, 0x0) != 0 || b > n)
		goto err;

	n = b;
	DEBUG("len %#hhx\n", n);

	// read checksum
	if(read(brdg, &csum, 1, 0x0) != 0)
		goto err;

	DEBUG("checksum %#hhx\n", csum);

	// read data
	chunksize = CHUNKSIZE(brdg->cfg->chunksize_e);
	chunks = CHUNKS(n, chunksize);
	DEBUG("payload: cs %u, chunks %u\n", chunksize, chunks);

	for(i=0; i<chunks; i++){
		if(read(brdg, data + i * chunksize, MIN(chunksize, n - (i * chunksize)), 0x0) != 0)
			goto err;

		for(uint8_t j=0; j<MIN(chunksize, n - (i * chunksize)); j++)
			DEBUG("data %#hhx\n", ((char*)data)[i * chunksize + j]);
	}

	// verify and acknowledge checksum
	b = checksum(data, n);
	DEBUG("verify %#hhx\n", b);

	if(write(brdg, &b, 1) != 0 || b != csum)
		goto err;

	brdg->seq_num++;
	DEBUG("read complete\n");

	return n;


err:
	brdg->seq_num = 0;

	return -1;
}

int16_t bridge_write(bridge_t *brdg, void const *data, uint8_t n){
	uint8_t i,
			chunks,
			chunksize,
			csum;


	// write control byte
	DEBUG("control byte %#hhx\n", ((brdg->seq_num << 3) | brdg->cfg->chunksize_e));

	if(write(brdg, (uint8_t []){ ((brdg->seq_num << 3) | brdg->cfg->chunksize_e) }, 1) != 0)
		goto err;

	// write data length
	DEBUG("len %#hhx\n", n);

	if(write(brdg, &n, 1) != 0)
		goto err;

	// write checksum
	csum = checksum(data, n);
	DEBUG("checksum %#hhx\n", csum);

	if(write(brdg, &csum, 1) != 0)
		goto err;

	// write data
	chunksize = CHUNKSIZE(brdg->cfg->chunksize_e);
	chunks = CHUNKS(n, chunksize);
	DEBUG("payload: cs %u, chunks %u\n", chunksize, chunks);

	for(i=0; i<chunks; i++){
		for(uint8_t j=0; j<MIN(chunksize, n - (i * chunksize)); j++)
			DEBUG("data %#hhx\n", ((char*)data)[i * chunksize + j]);

		if(write(brdg, data + i * chunksize, MIN(chunksize, n - (i * chunksize))) != 0)
			goto err;
	}

	// read acknowledge
	DEBUG("verify %#hhx\n", csum);

	if(read(brdg, &i, 1, &csum) != 0)
		goto err;

	brdg->seq_num++;
	DEBUG("write complete\n");

	return n;


err:
	brdg->seq_num = 0;

	return -1;
}


/* local functions */
static int read(bridge_t *brdg, uint8_t *data, uint8_t n, uint8_t *expect){
	size_t i;


	if(brdg->ops.read(brdg, data, n) != 0){
		DEBUG("read error\n");
		return nack(brdg, data[0]);
	}

	if(expect != 0x0){
		if(memcmp(data, expect, n) != 0){
			DEBUG("expect error\n");
			return nack(brdg, data[n - 1]);
		}
	}

	return ack(brdg, data[n - 1]);
}

static int write(bridge_t *brdg, uint8_t const *data, uint8_t n){
	uint8_t c;
	int r;


	r = brdg->ops.write(brdg, data, n);

	if(brdg->ops.read(brdg, &c, 1) != 0){
		DEBUG("read ack failed\n");
		return -1;
	}

	c = ~c;

	if(r != 0 || c != data[n - 1]){
		DEBUG("ack failed len %#hhx -- %#hhx, data %#hhx -- %#hhx\n", (uint8_t)r, n, c, data[n - 1]);
		return -1;
	}

	return 0;
}

static int ack(bridge_t *brdg, uint8_t byte){
	uint8_t x;

	x = ~byte;

	if(brdg->ops.write(brdg, &x, 1) != 0){
		DEBUG("ack failed %#hhx %#hhx\n", x, byte);
		return -1;
	}

	DEBUG("ack success %#hhx (%#hhx)\n", byte, ~byte);
	return 0;
//	return brdg->term->putc(~byte, brdg->term->data) == ~byte ? 0 : -1;
}

static int nack(bridge_t *brdg, uint8_t byte){
	DEBUG("nack\n");
	(void)ack(brdg, ~byte);

	return -1;
}

static uint8_t checksum(uint8_t const *data, size_t n){
	uint8_t s;
	size_t i;


	s = 0;

	for(i=0; i<n; i++)
		s += ~data[i] + 1;

	return s;
}

static void rx_hdlr(int_num_t num, void *brdg){
	// TODO
}

static void tx_hdlr(int_num_t num, void *brdg){
	// TODO
}
