/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/memory.h>
#include <stdlib.h>
#include <memory.h>


/* local/static prototypes */
static void *test_malloc(size_t size);
static void *test_calloc(size_t n, size_t size);


/* global variables */
size_t test_malloc_fail_at = 0;


/* global functions */
void test_memory_init(void){
	mallocp = test_malloc;
	callocp = test_calloc;
}

void test_memory_reset(void){
	mallocp = malloc;
	callocp = calloc;
}


/* local functions */
static void *test_malloc(size_t size){
	if(--test_malloc_fail_at == 0)
		return 0x0;
	return malloc(size);
}

static void *test_calloc(size_t n, size_t size){
	if(--test_malloc_fail_at == 0)
		return 0x0;
	return calloc(n, size);
}
