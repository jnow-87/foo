#ifndef ARCH_THREAD_H
#define ARCH_THREAD_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define thread_context_init(thread, proc_entry, thread_arg)	(arch_kernel_call(thread_context_init, 0x0)(thread, proc_entry, thread_arg))


#endif // ARCH_THREAD_H
