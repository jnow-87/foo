/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <kernel/kprintf.h>
#include <kernel/thread.h>
#include <kernel/sched.h>
#include <sys/ctype.h>
#include <sys/devicetree.h>
#include <sys/stack.h>
#include <sys/stdarg.h>
#include <sys/types.h>


/* macros */
#define STACK_CHUNK_SIZE	16


/* local/static prototypes */
void print_stack(void *addr);


/* global functions */
void kpanic_ext(char const *file, char const *func, unsigned int line, char const *format, ...){
	va_list lst;
	thread_t *this_t;


	int_enable(false);

	/* print woops */
	va_start(lst, format);

	kprintf(KMSG_ANY, FG("\n\nwoops!!", RED) "\t");
	kvprintf(KMSG_ANY, format, lst);

#ifdef DEVTREE_ARCH_MULTI_CORE
	kprintf(KMSG_ANY, "%10.10s: %u\n", "core", PIR);
#endif // DEVTREE_ARCH_MULTI_CORE

	kprintf(KMSG_ANY, "%10.10s: %s:%u %s()\n", "location", file, line, func);

	this_t = sched_running_nopanic();

	if(this_t){
		kprintf(KMSG_ANY, "%10.10s: %s(%u).%u\n\n",
			"thread",
			this_t->parent->name,
			(unsigned int)(this_t->parent->pid),
			(unsigned int)(this_t->tid)
		);
		print_stack(this_t->stack->phys_addr);
	}
	else
		kprintf(KMSG_ANY, "%10.10s: 0x0\n\n", "thread");

	va_end(lst);

	/* arch panic */
	core_panic(this_t ? stack_top(this_t->ctx_stack) : 0x0);

	/* halt core */
	while(1){
		core_sleep();
	}
}


/* local functions */
void print_stack(void *addr){
	uint8_t byte;


	for(size_t i=0; i<CONFIG_STACK_SIZE; i+=STACK_CHUNK_SIZE){
		kprintf(KMSG_ANY, "  %p\t", addr + i);

		for(size_t j=0; j<STACK_CHUNK_SIZE; j++)
			kprintf(KMSG_ANY, " %2.2hhx", *((uint8_t*)addr + i + j));

		kprintf(KMSG_ANY, "\t");

		for(size_t j=0; j<STACK_CHUNK_SIZE; j++){
			byte = *((uint8_t*)addr + i + j);
			kprintf(KMSG_ANY, "%c", isprint(byte) ? byte : '.');
		}

		kprintf(KMSG_ANY, "\n");
	}

	kprintf(KMSG_ANY, "\n");
}
