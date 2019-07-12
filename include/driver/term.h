/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_TERM_H
#define DRIVER_TERM_H


#include <config/config.h>
#include <arch/interrupt.h>
#include <kernel/ksignal.h>
#include <kernel/devfs.h>
#include <sys/ringbuf.h>
#include <sys/term.h>
#include <sys/mutex.h>


/* incomplete types */
struct term_t;


/* types */
typedef struct{
	int (*configure)(void *cfg, void *regs);

	term_flags_t (*get_flags)(void *cfg);

	void (*putc)(char c, void *regs);
	size_t (*puts)(char const *s, size_t n, void *regs);
	size_t (*gets)(char *s, size_t n, term_err_t *err, void *regs);

	void *regs;
	int_num_t rx_int,
			  tx_int;

	uint8_t cfg_size;
} term_itf_t;

typedef struct term_txqueue_t{
	struct term_txqueue_t *next;

	char const *data;
	size_t len;

	ksignal_t fin;
} term_txqueue_t;

typedef struct term_t{
	void *cfg;
	term_itf_t *hw;

	term_err_t rx_err;
	ringbuf_t rx_buf;
	ksignal_t *rx_rdy;

	mutex_t tx_mtx;
	term_txqueue_t *tx_queue_head,
				   *tx_queue_tail;
} term_t;


#endif // DRIVER_TERM_H
