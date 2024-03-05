/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/interrupt.h>
#include <kernel/memory.h>
#include <kernel/interrupt.h>
#include <kernel/inttask.h>
#include <kernel/ksignal.h>
#include <driver/term.h>
#include <sys/types.h>
#include <sys/ctype.h>
#include <sys/escape.h>
#include <sys/mutex.h>
#include <sys/ringbuf.h>
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

static char putc(term_t *term, char c);
static size_t puts(term_t *term, char const *s, size_t n, bool blocking);
static errno_t error(term_t *term);

static int tx_complete(void *payload);


/* global functions */
/**
 * \brief	allocate and initialise a terminal struct
 *
 * 			If the given terminal interface indicates enabled interrupts
 * 			the receive buffer is initiased accordingly.
 *
 * \param	itf		terminal hardware interface
 * \param	cfg		terminal configuration
 * \param	node	pointer to the associated devfs node
 *
 * \return	pointer to the terminal
 * 			0x0 on error
 */
term_t *term_create(term_itf_t *itf, term_cfg_t *cfg, fs_node_t *node){
	void *buf;
	term_t *term;


	if(itf->gets == 0x0 || itf->puts == 0x0)
		goto_errno(err_0, E_INVAL);

	/* allocate terminal */
	term = kcalloc(1, sizeof(term_t));

	if(term == 0x0)
		goto err_0;

	/* allocate recv buffer */
	buf = 0x0;

	if(itf->rx_int){
		buf = kmalloc(CONFIG_TERM_RXBUF_SIZE);

		if(buf == 0x0)
			goto err_1;
	}

	/* init term */
	term->cfg = cfg;
	term->node = node;
	term->itf = itf;
	term->errno = 0;

	esc_init(&term->esc);
	ringbuf_init(&term->rx_buf, buf, CONFIG_TERM_RXBUF_SIZE);
	itask_queue_init(&term->tx_queue);

	return term;


err_1:
	kfree(term);

err_0:
	return 0x0;
}

void term_destroy(term_t *term){
	kfree(term->rx_buf.buf);
	kfree(term);
}

size_t term_gets(term_t *term, char *s, size_t n){
	size_t r;


	r = term->itf->rx_int ? ringbuf_read(&term->rx_buf, s, n) : term->itf->gets(s, n, term->itf->hw);

	if(r == 0)
		term->errno = errno;

	return r;
}

size_t term_puts(term_t *term, char const *s, size_t n){
	size_t r;


	if(n == 0)
		return 0;

	r = (int_enabled() != INT_NONE && term->itf->tx_int)
	  ? puts_int(term, s, n)
	  : puts_poll(term, s, n, true)
	;

	if(r == 0)
		term->errno = errno;

	return term->errno ? 0 : r;
}

void term_rx_hdlr(int_num_t num, void *payload){
	term_t *term = (term_t*)payload;
	char buf[16];
	size_t n;


	mutex_lock(&term->node->mtx);

	n = term->itf->gets(buf, 16, term->itf->hw);

	if(n == 0)
		term->errno = errno;

	if(ringbuf_write(&term->rx_buf, buf, n) != n)
		term->errno = E_LIMIT;

	ksignal_send(&term->node->datain_sig);

	mutex_unlock(&term->node->mtx);
}

void term_tx_hdlr(int_num_t num, void *payload){
	term_t *term = (term_t*)payload;
	size_t n;
	tx_dgram_t *dgram;


	dgram = itask_query_payload(&term->tx_queue, tx_complete);

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


/* local functions */
static size_t puts_int(term_t *term, char const *s, size_t n){
	tx_dgram_t dgram;


	dgram.s = s;
	dgram.len = n;
	dgram.term = term;

	term->errno = itask_issue(&term->tx_queue, &dgram, term->itf->tx_int);

	return n - dgram.len;
}

static size_t puts_poll(term_t *term, char const *s, size_t n, bool blocking){
	size_t i,
		   j;


	if(!CANON(term))
		return term->itf->puts(s, n, blocking, term->itf->hw);

	if(term_cursor_show(term, false) != 0)
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

	(void)term_cursor_show(term, CURSOR(term));

	return j;
}

static char putc(term_t *term, char c){
	if(!CANON(term) || (!esc_active(&term->esc) && isprint(c))){
		if(term->itf->putc(c, term->itf->hw) != c || term_cursor_move(term, 0, 1, false) != 0)
			return ~c;

		return c;
	}

	return (term_esc_handle(term, c) == 0) ? c : ~c;
}

static size_t puts(term_t *term, char const *s, size_t n, bool blocking){
	n = term->itf->puts(s, n, blocking, term->itf->hw);

	return (term_cursor_move(term, 0, n, false) == 0) ? n : 0;
}

static errno_t error(term_t *term){
	return (term->itf->error != 0x0) ? term->itf->error(term->itf->hw) : 0;
}

static int tx_complete(void *payload){
	tx_dgram_t *dgram = (tx_dgram_t*)payload;
	errno_t ecode;


	ecode = error(dgram->term);

	if(ecode != 0)
		return ecode;

	return (dgram->len == 0) ? 0 : -1;
}
