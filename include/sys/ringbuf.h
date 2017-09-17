#ifndef SYS_RINGBUF_H
#define SYS_RINGBUF_H


#include <sys/types.h>


/* macros */
#define RINGBUF_INITIALISER(_data, _size) { \
	.rd = _size - 1, \
	.wr = 0, \
	.size = _size, \
	.data = _data, \
}


/* types */
typedef struct{
	size_t rd,
		   wr,
		   size;

	void *data;
} ringbuf_t;


/* prototypes */
void ringbuf_init(ringbuf_t *buf, void *data, size_t n);
size_t ringbuf_read(ringbuf_t *buf, void *data, size_t n);
size_t ringbuf_write(ringbuf_t *buf, void *data, size_t n);


#endif // SYS_RINGBUF_H
