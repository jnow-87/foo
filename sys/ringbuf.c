/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/ringbuf.h>
#include <sys/string.h>
#include <sys/math.h>


/* global functions */
void ringbuf_init(ringbuf_t *ring, void *buf, size_t n){
	*ring = RINGBUF_INITIALISER(buf, n);
}

size_t ringbuf_read(ringbuf_t *ring, void *buf, size_t n){
	size_t i;


	for(i=0; i<n && (ring->rd + 1) % ring->size != ring->wr; i++){
		ring->rd = (ring->rd + 1) % ring->size;
		((char*)buf)[i] = ((char*)ring->buf)[ring->rd];
	}

	return i;
}

size_t ringbuf_write(ringbuf_t *ring, void *buf, size_t n){
	size_t i;


	for(i=0; i<n && ring->wr != ring->rd; i++){
		((char*)ring->buf)[ring->wr] = ((char*)buf)[i];
		ring->wr = (ring->wr + 1) % ring->size;
	}

	return i;
}

size_t ringbuf_contains(ringbuf_t *ring){
	return ring->size - ringbuf_left(ring) - 1;
}

size_t ringbuf_left(ringbuf_t *ring){
	return (ring->rd - ring->wr) + (ring->rd < ring->wr ? ring->size : 0);
}

bool ringbuf_empty(ringbuf_t *ring){
	return ((ring->rd + 1) % ring->size != ring->wr) ? false : true;
}

bool ringbuf_full(ringbuf_t *ring){
	return ring->wr == ring->rd ? true : false;
}
