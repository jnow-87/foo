/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/x86/linux.h>
#include <sys/types.h>
#include <sys/compiler.h>
#include <sys/memblock.h>


/* prototypes */
void *gcov_malloc(size_t size);
void *gc_all(size_t size) __alias(gcov_malloc);
void gcov_free(void *ptr);
void gc_f(void *ptr) __alias(gcov_free);

void *gc_m(void *addr, size_t n, int prot, int flags, int fd, unsigned long int offset) __alias(gcov_mmap);


/* static variables */
static uint8_t gcov_blob[CONFIG_X86_GCOV_HEAP_SIZE];
static memblock_t *gcov_heap = 0x0;


/* global functions */
void *gcov_malloc(size_t size){
	static int initialised = 0;
	void *p;


	if(!initialised){
		gcov_heap = (void*)gcov_blob;
		memblock_init(gcov_heap, CONFIG_X86_GCOV_HEAP_SIZE);

		initialised = 1;
	}

	p = memblock_alloc(&gcov_heap, size, 8);

	if(p == 0x0)
		LNX_EEXIT("out of gcov heap\n");

	return p;
}

void gcov_free(void *ptr){
	memblock_free(&gcov_heap, ptr);
}

void *gcov_mmap(void *addr, size_t n, int prot, int flags, int fd, unsigned long int offset){
	return lnx_mmap(addr, n, prot, flags, fd, offset);
}
