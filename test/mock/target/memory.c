/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/types.h>
#include <test/memory.h>


/* prototypes */
void *__malloc(size_t size);


/* global functions */
void *malloc(size_t size){
	return MEMMOCK_ALLOC_FAIL(1, size) ? 0x0 : __malloc(size);
}
