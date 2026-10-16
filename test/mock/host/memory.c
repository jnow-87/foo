/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/types.h>
#include <test/memory.h>


/* prototypes */
void *__libc_malloc(size_t size);
void *__libc_calloc(size_t n, size_t size);


/* global functions */
void *malloc(size_t size){
	return MEMMOCK_ALLOC_FAIL(1, size) ? 0x0 : __libc_malloc(size);
}

void *calloc(size_t n, size_t size){
	return MEMMOCK_ALLOC_FAIL(n, size) ? 0x0 : __libc_calloc(n, size);
}
