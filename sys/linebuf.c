/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/math.h>
#include <sys/string.h>
#include <sys/linebuf.h>


/* global functions */
void linebuf_init(linebuf_t *buf, void *data, size_t n, size_t prefill){
	*buf = LINEBUF_INITIALISER(data, n, prefill);
}

void linebuf_reset(linebuf_t *buf){
	buf->rd = 0;
	buf->wr = 0;
}

size_t linebuf_read(linebuf_t *buf, void *data, size_t n){
	n = linebuf_peek(buf, data, n);
	buf->rd += n;

	return n;
}

size_t linebuf_peek(linebuf_t *buf, void *data, size_t n){
	n = MIN(n, buf->wr - buf->rd);
	memcpy(data, buf->data + buf->rd, n);

	return n;
}

size_t linebuf_write(linebuf_t *buf, void *data, size_t n){
	n = MIN(n, buf->size - buf->wr);
	memcpy(buf->data + buf->wr, data, n);
	buf->wr += n;

	return n;
}

size_t linebuf_contains(linebuf_t *buf){
	return buf->wr - buf->rd;
}

size_t linebuf_left(linebuf_t *buf){
	return buf->size - buf->wr;
}

bool linebuf_empty(linebuf_t *buf){
	return (linebuf_contains(buf) == 0);
}

bool linebuf_full(linebuf_t *buf){
	return (linebuf_left(buf) == 0);
}
