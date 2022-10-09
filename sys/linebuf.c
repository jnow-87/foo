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
void linebuf_init(linebuf_t *line, void *buf, size_t n, size_t prefill){
	*line = LINEBUF_INITIALISER(buf, n, prefill);
}

void linebuf_reset(linebuf_t *line){
	line->rd = 0;
	line->wr = 0;
}

size_t linebuf_read(linebuf_t *line, void *buf, size_t n){
	n = linebuf_peek(line, buf, n);
	line->rd += n;

	return n;
}

size_t linebuf_peek(linebuf_t *line, void *buf, size_t n){
	n = MIN(n, line->wr - line->rd);
	memcpy(buf, line->buf + line->rd, n);

	return n;
}

size_t linebuf_write(linebuf_t *line, void *buf, size_t n){
	n = MIN(n, line->size - line->wr);
	memcpy(line->buf + line->wr, buf, n);
	line->wr += n;

	return n;
}

size_t linebuf_contains(linebuf_t *line){
	return line->wr - line->rd;
}

size_t linebuf_left(linebuf_t *line){
	return line->size - line->wr;
}

bool linebuf_empty(linebuf_t *line){
	return (linebuf_contains(line) == 0);
}

bool linebuf_full(linebuf_t *line){
	return (linebuf_left(line) == 0);
}
