/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <kernel/driver.h>
#include <kernel/interrupt.h>
#include <kernel/inttask.h>
#include <kernel/ksignal.h>
#include <kernel/memory.h>
#include <driver/term.h>
#include <sys/ctype.h>
#include <sys/escape.h>
#include <sys/mutex.h>
#include <sys/ringbuf.h>
#include <sys/string.h>
#include <sys/types.h>
#include "term.h"


/* types */
typedef struct{
	char const *s;
	size_t len;

	term_t *term;
} tx_dgram_t;


/* local/static prototypes */
static size_t puts_int(term_t *term, char const *s, size_t n);
static size_t puts_poll(term_t *term, char const *s, size_t n, bool blocking);
static void rx_hdlr(int_num_t num, void *payload);
static void tx_hdlr(int_num_t num, void *payload);

static char putc(term_t *term, char c);
static size_t puts(term_t *term, char const *s, size_t n, bool blocking);
static errno_t error(term_t *term);

static int tx_complete(void *payload);


/* global functions */
size_t term_gets(term_t *term, char *s, size_t n){
	/* read */
	n = term->itf->rx_int ? ringbuf_read(&term->rx_buf, s, n) : term->itf->gets(s, n, term->itf->hw);

	if(n == 0)
		goto err;

	/* handle terminal flags */
	if(flags_apply(term, s, n, 1, TFT_I,  term->cfg->iflags) == 0x0)
		goto_errno(err, E_IO);

	if(flags_apply(term, s, n, n, TFT_L, term->cfg->lflags) == 0x0)
		goto_errno(err, E_IO);

	return n;


err:
	return 0;
}

size_t term_puts(term_t *term, char const *s, size_t n){
	size_t n_put;
	char flagged[n];


	memcpy(flagged, s, n);

	/* handle terminal iflags */
	s = flags_apply(term, flagged, n, 1, TFT_O, term->cfg->oflags);
	n_put = s - flagged;

	if(s == 0x0)
		return 0;

	/* perform write */
	return (puts_raw(term, s, n - n_put) + n_put == n) ? n : 0;
}

size_t puts_raw(term_t *term, char const *s, size_t n){
	if(n == 0)
		return 0;

	if(int_enabled() && term->itf->tx_int)
		return puts_int(term, s, n);

	return puts_poll(term, s, n, true);
}


/* local functions */
static void *probe(char const *name, void *dt_data, void *dt_itf){
	term_cfg_t *dtd = (term_cfg_t*)dt_data;
	term_itf_t *dti = (term_itf_t*)dt_itf;
	void *buf = 0x0;
	term_t *term;


	/* create terminal */
	term = kcalloc(1, sizeof(term_t));

	if(term == 0x0)
		goto err_0;

	// allocate recv buffer
	if(dti->rx_int){
		buf = kmalloc(CONFIG_TERM_RXBUF_SIZE);

		if(buf == 0x0)
			goto err_1;
	}

	ringbuf_init(&term->rx_buf, buf, CONFIG_TERM_RXBUF_SIZE);

	// init term
	term->cfg = dtd;
	term->itf = dti;

	esc_init(&term->esc);
	itask_queue_init(&term->tx_queue);

	/* register interrupt */
	if(dti->rx_int && int_register(dti->rx_int, rx_hdlr, term) != 0)
		goto err_2;

	if(dti->tx_int && int_register(dti->tx_int, tx_hdlr, term) != 0)
		goto err_3;

	/* configure hardware */
	if(dti->configure != 0x0 && dti->configure(term->cfg, dti->cfg, dti->hw) != 0)
		goto err_4;

	return term;


err_4:
	if(dti->tx_int)
		int_release(dti->tx_int);

err_3:
	if(dti->rx_int)
		int_release(dti->rx_int);

err_2:
	kfree(buf);

err_1:
	kfree(term);

err_0:
	return 0x0;
}

driver_probe("terminal", probe);

static size_t puts_int(term_t *term, char const *s, size_t n){
	tx_dgram_t dgram;


	dgram.s = s;
	dgram.len = n;
	dgram.term = term;

	itask_issue(&term->tx_queue, &dgram, term->itf->tx_int);

	return n - dgram.len;
}

static size_t puts_poll(term_t *term, char const *s, size_t n, bool blocking){
	size_t i,
		   j;


	if(!CANON(term))
		return term->itf->puts(s, n, blocking, term->itf->hw);

	if(cursor_show(term, false) != 0)
		return 0;

	for(i=0, j=0; i<n; i++){
		if(!isprint(s[i]) || esc_active(&term->esc) || term->cursor.column + (i - j) >= term->cfg->columns){
			j += puts(term, s + j, i - j, blocking);

			if(j != i || putc(term, s[i]) != s[i])
				break;

			j = i + 1;
		}
	}

	if(errno == 0 && error(term) == 0)
		j += puts(term, s + j, i - j, blocking);

	(void)cursor_show(term, CURSOR(term));

	return j;
}

static void rx_hdlr(int_num_t num, void *payload){
	term_t *term = (term_t*)payload;
	char buf[16];
	size_t n;


	mutex_lock(&term->node->mtx);

	n = term->itf->gets(buf, 16, term->itf->hw);

	if(ringbuf_write(&term->rx_buf, buf, n) != n)
		set_errno(E_LIMIT);

	ksignal_send(&term->node->datain_sig);

	mutex_unlock(&term->node->mtx);
}

static void tx_hdlr(int_num_t num, void *payload){
	term_t *term = (term_t*)payload;
	size_t n;
	tx_dgram_t *dgram;


	dgram = itask_payload(&term->tx_queue, tx_complete);

	if(dgram == 0x0)
		return;

	mutex_lock(&term->node->mtx);
	n = puts_poll(term, dgram->s, dgram->len, false);
	mutex_unlock(&term->node->mtx);

	if(n > 0){
		dgram->s += n;
		dgram->len -= n;
	}
	else
		itask_complete(&term->tx_queue, errno ? errno : E_IO);
}

static char putc(term_t *term, char c){
	if(!CANON(term) || (!esc_active(&term->esc) && isprint(c))){
		if(term->itf->puts(&c, 1, true, term->itf->hw) != 1 || cursor_move(term, 0, 1, false) != 0)
			return ~c;

		return c;
	}

	return (esc_handle(term, c) == 0) ? c : ~c;
}

static size_t puts(term_t *term, char const *s, size_t n, bool blocking){
	n = term->itf->puts(s, n, blocking, term->itf->hw);

	return (cursor_move(term, 0, n, false) == 0) ? n : 0;
}

static errno_t error(term_t *term){
	return (term->itf->error != 0x0) ? term->itf->error(term->itf->hw) : 0;
}

static int tx_complete(void *payload){
	tx_dgram_t *dgram = (tx_dgram_t*)payload;
	errno_t e;


	e = error(dgram->term);

	if(e != 0)
		return e;

	return (dgram->len == 0) ? 0 : -1;
}
