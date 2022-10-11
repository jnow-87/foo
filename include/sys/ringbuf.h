/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_RINGBUF_H
#define SYS_RINGBUF_H


#include <sys/types.h>


/* macros */
#define RINGBUF_INITIALISER(_buf, _size) (ringbuf_t){ \
	.rd = _size - 1, \
	.wr = 0, \
	.size = _size, \
	.buf = _buf, \
}


/* types */
typedef struct{
	size_t rd,
		   wr,
		   size;

	void *buf;
} ringbuf_t;


/* prototypes */
void ringbuf_init(ringbuf_t *ring, void *buf, size_t n);
size_t ringbuf_read(ringbuf_t *ring, void *buf, size_t n);
size_t ringbuf_write(ringbuf_t *ring, void *buf, size_t n);
size_t ringbuf_contains(ringbuf_t *ring);
size_t ringbuf_left(ringbuf_t *ring);
bool ringbuf_empty(ringbuf_t *ring);
bool ringbuf_full(ringbuf_t *ring);


#endif // SYS_RINGBUF_H
