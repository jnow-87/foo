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
#include <sys/list.h>


/* global functions */
void kpanic_ext(thread_t const *this_t, char const *file, char const *func, unsigned int line, char const *format, ...){
	va_list lst;


	int_enable(INT_NONE);

	va_start(lst, format);

	kprintf(KMSG_ANY, FG_RED "\n\nwoops!!" RESET_ATTR "\t");
	kvprintf(KMSG_ANY, format, lst);
	kprintf(KMSG_ANY,
		"\ntrace\n"
		"%20.20s: %u\n"
		"%20.20s: %u\n"
		"%20.20s: %s\n"
		"%20.20s: %s\n"
		"%20.20s: %s\n"
		"%20.20s: %u\n\n"
		,
		"pid", (this_t == 0x0) ? 0 : (unsigned int)(this_t->parent->pid),
		"tid", (this_t == 0x0) ? 0 : (unsigned int)(this_t->tid),
		"process", (this_t == 0x0) ? "unknown" : this_t->parent->name,
		"file", file,
		"function", func,
		"line", line);

	va_end(lst);

	core_panic((this_t == 0x0 || this_t->ctx_stack == 0x0) ? 0x0 : list_first(this_t->ctx_stack));
}
