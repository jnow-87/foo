#include <sys/ringbuf.h>
#include <sys/string.h>
#include <sys/math.h>


/* global functions */
void ringbuf_init(ringbuf_t *buf, void *data, size_t n){
	buf->rd = n - 1;
	buf->wr = 0;
	buf->data = data;
	buf->size = n;
}

size_t ringbuf_read(ringbuf_t *buf, void *data, size_t n){
	size_t i;


	for(i=0; i<n && (buf->rd + 1) % buf->size != buf->wr; i++){
		buf->rd = (buf->rd + 1) % buf->size;
		((char*)data)[i] = ((char*)buf->data)[buf->rd];
	}

	return i;
}

size_t ringbuf_write(ringbuf_t *buf, void *data, size_t n){
	size_t i;


	for(i=0; i<n && buf->wr != buf->rd; i++){
		((char*)buf->data)[buf->wr] = ((char*)data)[i];
		buf->wr = (buf->wr + 1) % buf->size;
	}

	return i;
}
