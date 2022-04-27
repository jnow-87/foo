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
#include <sys/mutex.h>
#include <sys/ringbuf.h>


/* types */
typedef struct{
	char const *s;
	size_t len;
} tx_data_t;


/* global functions */
/**
 * \brief	allocate and initialise a terminal struct
 *
 * 			If the given terminal interface indicates enabled interrupts
 * 			the receive buffer is initiased accordingly.
 *
 * \param	hw		terminal hardware interface
 * \param	cfg		terminal configuration
 * 					hw->cfg_size is expected to match the underlying type
 *
 * \return	pointer to the terminal
 * 			0x0 on error
 */
term_t *term_create(term_itf_t *hw, void *cfg, fs_node_t *node){
	void *buf;
	term_t *term;


	if(hw->configure == 0x0 || hw->gets == 0x0 || hw->puts == 0x0)
		goto_errno(err_0, E_INVAL);

	/* allocate terminal */
	term = kmalloc(sizeof(term_t));

	if(term == 0x0)
		goto err_0;

	/* allocate recv buffer */
	buf = 0x0;

	if(hw->rx_int){
		buf = kmalloc(CONFIG_TERM_RXBUF_SIZE);

		if(buf == 0x0)
			goto err_1;
	}

	/* init term */
	term->cfg = cfg;
	term->hw = hw;
	term->node = node;
	term->errno = E_OK;

	ringbuf_init(&term->rx_buf, buf, CONFIG_TERM_RXBUF_SIZE);
	itask_queue_init(&term->tx_queue);

	return term;


err_2:
	kfree(buf);

err_1:
	kfree(term);

err_0:
	return 0x0;
}

void term_destroy(term_t *term){
	kfree(term->rx_buf.data);
	kfree(term);
}

size_t term_gets(term_t *term, char *s, size_t n){
	size_t r;


	r = term->hw->rx_int ? ringbuf_read(&term->rx_buf, s, n) : term->hw->gets(s, n, term->hw->data);

	if(r == 0)
		term->errno = errno;

	return r;
}

size_t term_puts(term_t *term, char const *s, size_t n){
	size_t r;
	tx_data_t data;


	if(n == 0)
		return 0;

	if(int_enabled() != INT_NONE && term->hw->tx_int){
		data.s = s;
		data.len = n;

		term->errno = itask_issue(&term->tx_queue, &data, term->hw->tx_int);
		r = n - data.len;
	}
	else
		r = term->hw->puts(s, n, term->hw->data);

	if(r == 0)
		term->errno = errno;

	return term->errno ? 0 : r;
}

term_flags_t *term_flags(term_t *term){
	return (term_flags_t*)(term->cfg + term->hw->cfg_flags_offset);
}

void term_rx_hdlr(int_num_t num, void *_term){
	char buf[16];
	size_t n;
	term_t *term;


	term = (term_t*)_term;

	mutex_lock(&term->node->mtx);

	n = term->hw->gets(buf, 16, term->hw->data);

	if(n == 0)
		term->errno = errno;

	if(ringbuf_write(&term->rx_buf, buf, n) != n)
		term->errno = E_LIMIT;

	ksignal_send(&term->node->datain_sig);

	mutex_unlock(&term->node->mtx);
}

void term_tx_hdlr(int_num_t num, void *_term){
	term_t *term;
	tx_data_t *data;


	term = (term_t*)_term;

	data = itask_query_data(&term->tx_queue);

	if(data == 0x0)
		return;

	mutex_lock(&term->node->mtx);
	while(term->hw->putc(*data->s, term->hw->data) != *data->s);
	mutex_unlock(&term->node->mtx);

	data->s++;
	data->len--;

	if(data->len == 0)
		itask_complete(&term->tx_queue, E_OK);
}
