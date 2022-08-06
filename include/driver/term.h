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
#include <kernel/fs.h>
#include <sys/errno.h>
#include <sys/mutex.h>
#include <sys/ringbuf.h>
#include <sys/term.h>
#include <sys/types.h>


/* macros */
#define TERM_FLAG(term, type, flag)	((term)->cfg->type & (flag))

#define TERM_IFLAG(term, flag)	TERM_FLAG(term, iflags, flag)
#define TERM_OFLAG(term, flag)	TERM_FLAG(term, oflags, flag)
#define TERM_LFLAG(term, flag)	TERM_FLAG(term, lflags, flag)

#define CANON(term)		TERM_LFLAG(term, TLFL_CANON)
#define WRAP(term)		TERM_LFLAG(term, TLFL_WRAP)
#define SCROLL(term)	TERM_LFLAG(term, TLFL_SCROLL)
#define CURSOR(term)	TERM_LFLAG(term, TLFL_CURSOR)


/* incomplete types */
struct term_t;


/* types */
typedef void * (*esc_hdlr_t)(struct term_t *term, char c);

typedef enum{
	TFT_I = 0,
	TFT_O,
	TFT_L,
} term_flag_type_t;

typedef enum{
	TE_MULTILINE = 0x1,
	TE_TO_START = 0x2,
	TE_TO_END = 0x4,
} term_erase_t;

typedef struct{
	void * (*hdlr)(struct term_t *term, char c);

	unsigned int val[2];
	uint8_t nval;
} term_esc_t;

typedef struct{
	uint16_t line,
			 column,
			 save_lime,
			 save_column;

	bool visible;
} term_cursor_t;

typedef struct{
	/* callbacks */
	int (*configure)(term_cfg_t *term_cfg, void *hw_cfg, void *data);

	char (*putc)(char c, void *data);
	size_t (*puts)(char const *s, size_t n, void *data);
	size_t (*gets)(char *s, size_t n, void *data);

	int (*cursor)(uint16_t line, uint16_t column, bool toggle, void *data);
	int (*scroll)(int16_t lines, void *data);
	int (*erase)(term_erase_t type, uint16_t n, void *hw);

	errno_t (*error)(void *data);

	/* hardware description */
	void *data,
		 *cfg;
	uint8_t cfg_size;

	int_num_t rx_int,
			  tx_int;
} term_itf_t;

typedef struct term_t{
	term_cfg_t *cfg;
	fs_node_t *node;

	term_itf_t *hw;

	ringbuf_t rx_buf;
	itask_queue_t tx_queue;

	errno_t errno;

	term_cursor_t cursor;
	term_esc_t esc;
} term_t;


/* prototypes */
term_t *term_create(term_itf_t *hw, term_cfg_t *cfg, fs_node_t *node);
void term_destroy(term_t *term);

size_t term_gets(term_t *term, char *s, size_t n);
size_t term_puts(term_t *term, char const *s, size_t n);

void term_rx_hdlr(int_num_t num, void *term);
void term_tx_hdlr(int_num_t num, void *term);

char *term_flags_apply(term_t *term, char *s, size_t n, size_t incr, term_flag_type_t fl_type, uint8_t flags);


#endif // DRIVER_TERM_H
