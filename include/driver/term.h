/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_TERM_H
#define DRIVER_TERM_H


#include <kernel/interrupt.h>
#include <kernel/inttask.h>
#include <kernel/ksignal.h>
#include <sys/mutex.h>
#include <sys/ringbuf.h>
#include <sys/term.h>
#include <sys/types.h>


/* types */
typedef enum{
	TFT_I = 0,
	TFT_O,
	TFT_L,
} term_flag_type_t;

typedef struct{
	int (*configure)(void *cfg, void *data);

	char (*putc)(char c, void *data);
	size_t (*puts)(char const *s, size_t n, void *data);
	size_t (*gets)(char *s, size_t n, term_err_t *err, void *data);

	void *data;
	int_num_t rx_int,
			  tx_int;

	uint8_t cfg_size,
			cfg_flags_offset;
} term_itf_t;

typedef struct{
	void *cfg;
	term_itf_t *hw;

	ringbuf_t rx_buf;
	ksignal_t *rx_rdy;
	term_err_t rx_err;

	itask_queue_t tx_queue;
	mutex_t mtx;
} term_t;


/* prototypes */
term_t *term_create(term_itf_t *hw, void *cfg);
void term_destroy(term_t *term);

size_t term_gets(term_t *term, char *s, size_t n);
size_t term_puts(term_t *term, char const *s, size_t n);

term_flags_t *term_flags(term_t *term);

void term_rx_hdlr(int_num_t num, void *term);
void term_tx_hdlr(int_num_t num, void *term);

char *term_flags_apply(term_t *term, char *s, size_t n, size_t incr, term_flag_type_t fl_type, uint8_t flags);


#endif // DRIVER_TERM_H
