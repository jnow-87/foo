/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/memory.h>
#include <kernel/panic.h>
#include <sys/compiler.h>
#include <sys/string.h>
#include <sys/types.h>


/* macros */
#define STACK_MAGIC		0xb0
#define STACK_SAVE_AREA	32

STATIC_ASSERT(CONFIG_STACK_SIZE > STACK_SAVE_AREA);


/* global functions */
void memcheck_stack_prime(page_t *stack){
	uint8_t *addr = stack->phys_addr;


	memset(addr, 0, STACK_SAVE_AREA);
	*addr = STACK_MAGIC;
}

void memcheck_stack_check(page_t *stack){
	uint8_t *addr = stack->phys_addr;


	if(addr[0] != STACK_MAGIC)
		goto panic;

	for(size_t i=1; i<STACK_SAVE_AREA; i++){
		if(addr[i] != 0)
			goto panic;
	}

	return;

panic:
	kpanic("stack overflow\n");
}
