/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/interrupt.h>
#include <arch/core.h>
#include <kernel/kprintf.h>
#include <kernel/thread.h>
#include <kernel/sched.h>
#include <sys/stdarg.h>
#include <sys/stack.h>


/* global functions */
void kpanic_ext(char const *file, char const *func, unsigned int line, char const *format, ...){
	va_list lst;
	thread_t *this_t;


	int_enable(INT_NONE);

	/* print woops */
	va_start(lst, format);

	kprintf(KMSG_ANY, FG_RED "\n\nwoops!!" RESET_ATTR "\t");
	kvprintf(KMSG_ANY, format, lst);
	kprintf(KMSG_ANY, "%10.10s: %s:%u %s()\n", "location", file, line, func);

	this_t = sched_running_nopanic();

	if(this_t){
		kprintf(KMSG_ANY, "%10.10s: %s(%u).%u\n\n",
			"thread",
			this_t->parent->name,
			(unsigned int)(this_t->parent->pid),
			(unsigned int)(this_t->tid)
		);
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
