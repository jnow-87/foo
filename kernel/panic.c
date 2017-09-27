#include <arch/interrupt.h>
#include <arch/core.h>
#include <kernel/kprintf.h>
#include <kernel/sched.h>
#include <sys/stdarg.h>


/* global functions */
void _kernel_panic(char const *file, char const *func, unsigned int line, char const *format, ...){
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
		"pid", (unsigned int)(current_thread[PIR]->parent->pid),
		"tid", (unsigned int)(current_thread[PIR]->tid),
		"process", current_thread[PIR]->parent->name,
		"file", file,
		"function", func,
		"line", line);

	va_end(lst);

	core_panic();
}
