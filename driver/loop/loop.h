/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_LOOP_H
#define DRIVER_LOOP_H


#include <sys/ringbuf.h>


/* types */
typedef ringbuf_t loop_t;

typedef struct{
	uint16_t buf_len;
} loop_dt_data_t;


/* prototypes */
loop_t *loop_create(loop_dt_data_t *dt_data);
void loop_destroy(loop_t *loop);

size_t loop_read(loop_t *loop, void *buf, size_t n);
size_t loop_write(loop_t *loop, void *buf, size_t n);


#endif // DRIVER_LOOP_H
