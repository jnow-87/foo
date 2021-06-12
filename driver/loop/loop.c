/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/memory.h>
#include <sys/errno.h>
#include <sys/ringbuf.h>
#include "loop.h"


/* global functions */
loop_t *loop_create(loop_dt_data_t *dt_data){
	loop_t *loop;


	loop = kmalloc(sizeof(loop_t) + dt_data->buf_len);

	if(loop == 0x0)
		return 0x0;

	ringbuf_init((ringbuf_t*)loop, (char*)loop + sizeof(ringbuf_t), dt_data->buf_len);

	return loop;
}

void loop_destroy(loop_t *loop){
	kfree(loop);
}

size_t loop_read(loop_t *loop, void *buf, size_t n){
	n = ringbuf_read(loop, buf, n);

	if(n == 0)
		goto_errno(err, E_END);

	return n;


err:
	return 0;
}

size_t loop_write(loop_t *loop, void *buf, size_t n){
	return ringbuf_write(loop, buf, n);
}
