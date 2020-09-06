/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/memory.h>
#include <stdlib.h>
#include <testing/memory.h>


/* global variables */
size_t tmalloc_fail_at = 0;


/* global functions */
void tmemory_init(void){
	mallocp = tmalloc;
	callocp = tcalloc;
}

void tmemory_reset(void){
	mallocp = malloc;
	callocp = calloc;
}

void *tmalloc(size_t size){
	if(--tmalloc_fail_at == 0)
		return 0x0;
	return malloc(size);
}

void *tcalloc(size_t n, size_t size){
	if(--tmalloc_fail_at == 0)
		return 0x0;
	return calloc(n, size);
}
