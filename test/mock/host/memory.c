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
	if(memmock_alloc_fail < 0 || memmock_alloc_fail-- > 0)
		return __libc_malloc(size);

	return 0x0;
}

void *calloc(size_t n, size_t size){
	if(memmock_alloc_fail < 0 || memmock_alloc_fail-- > 0)
		return __libc_calloc(n, size);

	return 0x0;
}
