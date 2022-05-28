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
#define LINEBUF_INITIALISER(_data, _size, _prefill) (linebuf_t){ \
	.rd = 0, \
	.wr = _prefill, \
	.size = _size, \
	.data = _data, \
}


/* types */
typedef struct{
	size_t rd,
		   wr,
		   size;

	void *data;
} linebuf_t;


/* prototypes */
void linebuf_init(linebuf_t *buf, void *data, size_t n, size_t prefill);
void linebuf_reset(linebuf_t *buf);

size_t linebuf_read(linebuf_t *buf, void *data, size_t n);
size_t linebuf_peek(linebuf_t *buf, void *data, size_t n);
size_t linebuf_write(linebuf_t *buf, void *data, size_t n);

size_t linebuf_contains(linebuf_t *buf);
size_t linebuf_left(linebuf_t *buf);
bool linebuf_empty(linebuf_t *buf);
bool linebuf_full(linebuf_t *buf);


#endif // SYS_LINEBUF_H
