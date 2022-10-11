/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_LINEBUF_H
#define SYS_LINEBUF_H


#include <sys/types.h>


/* macros */
#define LINEBUF_INITIALISER(_buf, _size, _prefill) (linebuf_t){ \
	.rd = 0, \
	.wr = _prefill, \
	.size = _size, \
	.buf = _buf, \
}


/* types */
typedef struct{
	size_t rd,
		   wr,
		   size;

	void *buf;
} linebuf_t;


/* prototypes */
void linebuf_init(linebuf_t *line, void *buf, size_t n, size_t prefill);
void linebuf_reset(linebuf_t *line);

size_t linebuf_read(linebuf_t *line, void *buf, size_t n);
size_t linebuf_peek(linebuf_t *line, void *buf, size_t n);
size_t linebuf_write(linebuf_t *line, void *buf, size_t n);

size_t linebuf_contains(linebuf_t *line);
size_t linebuf_left(linebuf_t *line);
bool linebuf_empty(linebuf_t *line);
bool linebuf_full(linebuf_t *line);


#endif // SYS_LINEBUF_H
