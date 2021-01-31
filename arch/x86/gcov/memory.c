/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <sys/types.h>
#include <sys/compiler.h>


/* macros */
#define GCOV_HEAP_SIZE	512


/* prototypes */
void *gcov_malloc(size_t size);
void *gc_all(size_t size) __alias(gcov_malloc);
void gcov_free(void *ptr);
void gc_f(void *ptr) __alias(gcov_free);


/* static variables */
static uint8_t gcov_heap[GCOV_HEAP_SIZE];
static bool gcov_heap_inuse = false;


/* global functions */
void *gcov_malloc(size_t size){
	if(gcov_heap_inuse)
		LNX_EEXIT("supporting at most one allocation\n");

	if(size > GCOV_HEAP_SIZE)
		LNX_EEXIT("heap too small, %u > %u\n", size, GCOV_HEAP_SIZE);

	gcov_heap_inuse = true;

	return gcov_heap;
}

void gcov_free(void *ptr){
	gcov_heap_inuse = false;
}
