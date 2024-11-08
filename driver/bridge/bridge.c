/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/driver.h>
#include <kernel/interrupt.h>
#include <kernel/kprintf.h>
#include <kernel/memory.h>
#include <driver/bridge.h>
#include <sys/types.h>
#include <sys/math.h>
#include <sys/string.h>
#include <sys/errno.h>
#include <sys/mutex.h>
#include <sys/list.h>
#include "dgram.h"


/* local/static prototypes */
static int16_t rw(bridge_t *brdg, void *buf, uint8_t n, bridge_dgram_type_t type);

static int16_t read_int(bridge_t *brdg, void *buf, uint8_t n);
static int16_t write_int(bridge_t *brdg, void *buf, uint8_t n);
static int16_t poll(bridge_t *brdg, void *buf, uint8_t n, bridge_dgram_type_t type);

static void int_hdlr(int_num_t num, void *payload);
static int int_rx(bridge_t *brdg, bridge_dgram_t *dgram);
static int int_tx(bridge_t *brdg, bridge_dgram_t *dgram);

static bridge_dgram_t *dgram_select(bridge_t *brdg, int_num_t num);


/* static variables */
static bridge_t *bridge_lst = 0x0;
static mutex_t bridge_mtx = MUTEX_INITIALISER();


/* global functions */
bridge_t *bridge_create(bridge_ops_t *ops, bridge_cfg_t *cfg, void *hw){
	bridge_t *brdg;


	/* allocate bridge */
	brdg = kcalloc(1, sizeof(bridge_t));

	if(brdg == 0x0)
		goto err_0;

	brdg->cfg = cfg;
	brdg->hw = hw;

	mutex_init(&brdg->mtx, MTX_NOINT);

	if(ops != 0x0)
		brdg->ops = *ops;

	/* link bridge peers */
	mutex_lock(&bridge_mtx);

	list_for_each(bridge_lst, brdg->peer){
		if(brdg->peer->cfg->id == cfg->id)
			break;
	}

	list_add_tail(bridge_lst, brdg);

	mutex_unlock(&bridge_mtx);

	if(brdg->peer){
		if(brdg->peer->peer != 0x0){
			FATAL("bridge id %hhu used more than twice\n", cfg->id);
			goto_errno(err_1, E_INUSE);
		}

		MUTEXED(&brdg->peer->mtx, brdg->peer->peer = brdg);
	}


	/* init interrupts */
	if(ops && cfg->rx_int && int_register(cfg->rx_int, int_hdlr, brdg) != 0)
		goto err_1;

	if(ops && cfg->tx_int && int_register(cfg->tx_int, int_hdlr, brdg) != 0)
		goto err_1;

	return brdg;


err_1:
	bridge_destroy(brdg);

err_0:
	return 0x0;
}

void bridge_destroy(bridge_t *brdg){
	bridge_cfg_t *cfg = brdg->cfg;
	bridge_dgram_t *dgram;


	list_rm_safe(bridge_lst, brdg, &bridge_mtx);

	if(cfg->rx_int)
		int_release(cfg->rx_int);

	if(cfg->tx_int)
		int_release(cfg->tx_int);

	list_for_each(brdg->rx_dgrams, dgram)
		dgram_free(dgram, brdg);

	list_for_each(brdg->tx_dgrams, dgram)
		dgram_free(dgram, brdg);

	if(brdg->peer)
		MUTEXED(&brdg->peer->mtx, brdg->peer->peer = 0x0);

	kfree(brdg);
}

int16_t bridge_read(bridge_t *brdg, void *buf, uint8_t n){
	return rw(brdg, buf, n, BDT_READ);
}

int16_t bridge_write(bridge_t *brdg, void *buf, uint8_t n){
	return rw(brdg, buf, n, BDT_WRITE);
}


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	return bridge_create(0x0, dt_data, 0x0);
}

driver_probe("bridge,itf", probe);

static int16_t rw(bridge_t *brdg, void *buf, uint8_t n, bridge_dgram_type_t type){
	int16_t r = -1;
	bridge_t *peer;


	mutex_lock(&brdg->mtx);

	peer = brdg->peer;

	if(peer == 0x0)
		goto_errno(end, E_NOCONN);

	if(!callbacks_set(&peer->ops, bridge_ops_t))
		goto_errno(end, E_NOIMP);

	if(type == BDT_READ && peer->cfg->rx_int)		r = read_int(peer, buf, n);
	else if(type == BDT_WRITE && peer->cfg->tx_int)	r = write_int(peer, buf, n);
	else											r = poll(peer, buf, n, type);

end:
	mutex_unlock(&brdg->mtx);

	if(r < 0)
		return_errno(errno ? errno : E_IO);

	return r;
}

static int16_t read_int(bridge_t *brdg, void *buf, uint8_t n){
	uint8_t i,
			x;
	bridge_dgram_t *dgram;


	for(i=0; i<n; i+=x){
		dgram = list_first(brdg->rx_dgrams);

		if(dgram == 0x0 || dgram->state != BDS_COMPLETE)
			break;

		x = MIN(n - i, dgram->len - dgram->offset);
		memcpy(buf + i, dgram->buf + dgram->offset, x);

		dgram->offset += x;

		if(dgram->offset == dgram->len)
			dgram_free(dgram, brdg);
	}

	return i;
}

static int16_t write_int(bridge_t *brdg, void *buf, uint8_t n){
	bridge_dgram_t *dgram;


	dgram = dgram_alloc_tx(brdg, buf, n);

	if(dgram != 0x0 && brdg->rx_dgrams == 0x0 && brdg->tx_dgrams == dgram)
		int_foretell(brdg->cfg->tx_int);

	return (dgram == 0x0) ? -1 : n;
}

static int16_t poll(bridge_t *brdg, void *buf, uint8_t n, bridge_dgram_type_t type){
	bridge_dgram_t dgram;
	bridge_dgram_state_t s;
	bridge_dgram_state_t (*op)(bridge_dgram_t *, bridge_t *);


	op = (type == BDT_WRITE) ? dgram_write : dgram_read;
	dgram_init(&dgram, type, buf, n, brdg);

	while(1){
		s = op(&dgram, brdg);

		if(s == BDS_COMPLETE)
			return dgram.len;

		dgram.state = s;

		if(s == BDS_ERROR && dgram_init_retry(&dgram) != 0)
			return -1;
	}
}

static void int_hdlr(int_num_t num, void *payload){
	bridge_t *brdg = (bridge_t*)payload;
	bridge_dgram_t *dgram;
	int_num_t cplt_int;


	mutex_lock(&brdg->mtx);

	dgram = dgram_select(brdg, num);

	if(dgram == 0x0)
		goto unlock;

	/* handle interrupt */
	if((dgram->type == BDT_READ) ? int_rx(brdg, dgram) : int_tx(brdg, dgram) != 0)
		goto unlock;

	/* transfer complete */
	cplt_int = (dgram->type == BDT_READ) ? brdg->peer->cfg->rx_int : brdg->peer->cfg->tx_int;

	// notify bridge peer
	if(cplt_int)
		int_foretell(cplt_int);

	// trigger next tx
	if(!list_empty(brdg->tx_dgrams))
		int_foretell(brdg->cfg->tx_int);

unlock:
	mutex_unlock(&brdg->mtx);
}

static int int_rx(bridge_t *brdg, bridge_dgram_t *dgram){
	/* protocol action */
	dgram->state = dgram_read(dgram, brdg);

	/* error handling */
	if(dgram->state != BDS_COMPLETE){
		if(dgram->state == BDS_ERROR)
			dgram_free(dgram, brdg);

		return 1;
	}

	/* transfer complete */
	dgram->offset = 0;

	return 0;
}

static int int_tx(bridge_t *brdg, bridge_dgram_t *dgram){
	/* protocol action */
	dgram->state = dgram_write(dgram, brdg);

	/* error handling */
	if(dgram->state != BDS_ERROR && dgram->state != BDS_COMPLETE)
		return 1;

	if(dgram->state == BDS_ERROR){
		// trigger retry
		if(dgram_init_retry(dgram) == 0)
			return 0;

		brdg->errnum = dgram_errno(dgram);
	}

	/* transfer complete */
	dgram_free(dgram, brdg);

	return 0;
}

static bridge_dgram_t *dgram_select(bridge_t *brdg, int_num_t num){
	bridge_dgram_t *tx,
				   *rx;


	tx = list_first(brdg->tx_dgrams);
	rx = list_last(brdg->rx_dgrams);

	if(tx == 0x0 && (rx == 0x0 || rx->state == BDS_COMPLETE)){
		if(num != brdg->cfg->rx_int)
			return 0x0;

		return dgram_alloc_rx(brdg);
	}

	if(tx == 0x0 || (rx != 0x0 && rx->state != BDS_CTRL_BYTE && rx->state != BDS_COMPLETE))
		return rx;

	return tx;
}
