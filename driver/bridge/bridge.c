/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/interrupt.h>
#include <kernel/memory.h>
#include <kernel/kprintf.h>
#include <driver/bridge.h>
#include <sys/types.h>
#include <sys/math.h>
#include <sys/string.h>
#include <sys/errno.h>
#include <sys/mutex.h>
#include <sys/list.h>
#include "dgram.h"


/* local/static prototypes */
static int16_t read_int(bridge_t *brdg, void *data, uint8_t n);
static int16_t write_int(bridge_t *brdg, void const *data, uint8_t n);
static int16_t poll(bridge_t *brdg, void const *data, uint8_t n, bridge_dgram_type_t type);

static void int_hdlr(int_num_t num, void *brdg);
static int_num_t int_rx(bridge_t *brdg, bridge_dgram_t *dgram);
static int_num_t int_tx(bridge_t *brdg, bridge_dgram_t *dgram);

static bridge_dgram_t *dgram_select(bridge_t *brdg, int_num_t num);


/* static variables */
static bridge_t *bridge_lst = 0x0;
static mutex_t bridge_mtx = MUTEX_INITIALISER();


/* global functions */
bridge_t *bridge_create(bridge_cfg_t *cfg, bridge_ops_t *ops, void *hw){
	bridge_t *brdg;


	/* allocate bridge */
	brdg = kcalloc(1, sizeof(bridge_t));

	if(brdg == 0x0)
		goto err_0;

	brdg->cfg = cfg;
	brdg->ops = *ops;
	brdg->hw = hw;
	mutex_init(&brdg->mtx, MTX_NOINT);

	/* init interrupts */
	if(hw && cfg->rx_int && int_register(cfg->rx_int, int_hdlr, brdg) != 0)
		goto err_1;

	if(hw && cfg->tx_int && int_register(cfg->tx_int, int_hdlr, brdg) != 0)
		goto err_1;

	/* link bridge peers */
	mutex_lock(&bridge_mtx);

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

	list_add_tail(bridge_lst, brdg);

	mutex_unlock(&bridge_mtx);

	return brdg;


err_1:
	bridge_destroy(brdg);

err_0:
	return 0x0;
}

void bridge_destroy(bridge_t *brdg){
	bridge_cfg_t *cfg;
	bridge_dgram_t *dgram;


	cfg = brdg->cfg;

	if(cfg->rx_int)
		int_release(cfg->rx_int);

	if(cfg->tx_int)
		int_release(cfg->tx_int);

	list_for_each(brdg->rx_dgrams, dgram)
		dgram_free(dgram, brdg);

	list_for_each(brdg->tx_dgrams, dgram)
		dgram_free(dgram, brdg);

	brdg->peer->peer = 0x0;
	kfree(brdg);
}

int16_t bridge_read(bridge_t *brdg, void *data, uint8_t n){
	return brdg->cfg->rx_int ? read_int(brdg, data, n) : poll(brdg, data, n, DT_READ);
}

int16_t bridge_write(bridge_t *brdg, void const *data, uint8_t n){
	return brdg->cfg->tx_int ? write_int(brdg, data, n) : poll(brdg, data, n, DT_WRITE);
}


/* local functions */
static int16_t read_int(bridge_t *brdg, void *data, uint8_t n){
	uint8_t i,
			x;
	bridge_dgram_t *dgram;


	mutex_lock(&brdg->mtx);

	for(i=0; i<n; i+=x){
		dgram = list_first(brdg->rx_dgrams);

		if(dgram == 0x0 || dgram->state != DS_COMPLETE)
			break;

		x = MIN(n - i, dgram->len - dgram->offset);
		memcpy(data + i, dgram->data + dgram->offset, x);

		dgram->offset += x;

		if(dgram->offset == dgram->len)
			dgram_free(dgram, brdg);
	}

	mutex_unlock(&brdg->mtx);

	return i;
}

static int16_t write_int(bridge_t *brdg, void const *data, uint8_t n){
	bridge_dgram_t *dgram;


	mutex_lock(&brdg->mtx);

	dgram = dgram_alloc_tx(brdg, data, n);

	if(dgram != 0x0 && brdg->rx_dgrams == 0x0 && brdg->tx_dgrams == dgram)
		int_foretell(brdg->cfg->tx_int);

	mutex_unlock(&brdg->mtx);

	return (dgram == 0x0) ? -1 : n;
}

static int16_t poll(bridge_t *brdg, void const *data, uint8_t n, bridge_dgram_type_t type){
	bridge_dgram_t dgram;
	bridge_dgram_state_t s;
	bridge_dgram_state_t (*op)(bridge_dgram_t *, bridge_t *);


	op = (type == DT_WRITE) ? dgram_write : dgram_read;
	dgram_init(&dgram, type, (void*)data, n, brdg);

	while(1){
		mutex_lock(&brdg->mtx);
		s = op(&dgram, brdg);
		mutex_unlock(&brdg->mtx);

		if(s == DS_COMPLETE)
			return dgram.len;

		dgram.state = s;

		if(s == DS_ERROR && dgram_init_retry(&dgram) != 0)
			return -1;
	}
}

static void int_hdlr(int_num_t num, void *_brdg){
	bridge_t *brdg;
	bridge_dgram_t *dgram;
	int_num_t cplt_int;


	brdg = (bridge_t*)_brdg;

	mutex_lock(&brdg->mtx);

	dgram = dgram_select(brdg, num);

	if(dgram != 0x0){
		cplt_int = (dgram->type == DT_READ) ? int_rx(brdg, dgram) : int_tx(brdg, dgram);

		/* transfer complete */
		if(cplt_int){
			// notify bridge peer
			int_foretell(cplt_int);

			// trigger next tx
			if(!list_empty(brdg->tx_dgrams))
				int_foretell(brdg->cfg->tx_int);
		}
	}

	mutex_unlock(&brdg->mtx);
}

static int_num_t int_rx(bridge_t *brdg, bridge_dgram_t *dgram){
	/* protocol action */
	dgram->state = dgram_read(dgram, brdg);

	/* error handling */
	if(dgram->state != DS_COMPLETE){
		if(dgram->state == DS_ERROR)
			dgram_free(dgram, brdg);

		return 0;
	}

	/* transfer complete */
	dgram->offset = 0;

	return brdg->peer->cfg->rx_int;
}

static int_num_t int_tx(bridge_t *brdg, bridge_dgram_t *dgram){
	/* protocol action */
	dgram->state = dgram_write(dgram, brdg);

	/* error handling */
	if(dgram->state != DS_ERROR && dgram->state != DS_COMPLETE)
		return 0;

	if(dgram->state == DS_ERROR){
		// trigger retry
		if(dgram_init_retry(dgram) == 0){
			int_foretell(brdg->cfg->tx_int);

			return 0;
		}

		brdg->errno = dgram_errno(dgram);
	}

	/* transfer complete */
	dgram_free(dgram, brdg);

	return brdg->peer->cfg->tx_int;
}

static bridge_dgram_t *dgram_select(bridge_t *brdg, int_num_t num){
	bridge_dgram_t *tx,
				   *rx;


	tx = list_first(brdg->tx_dgrams);
	rx = list_last(brdg->rx_dgrams);

	if(tx == 0x0 && (rx == 0x0 || rx->state == DS_COMPLETE)){
		if(num != brdg->cfg->rx_int)
			return 0x0;

		return dgram_alloc_rx(brdg);
	}

	if(tx == 0x0 || (rx != 0x0 && rx->state != DS_CTRL_BYTE && rx->state != DS_COMPLETE))
		return rx;

	return tx;
}
