/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_THREAD_H
#define ARCH_THREAD_H


#include <arch/arch.h>
#include <kernel/thread.h>
#include <sys/errno.h>


/* macros */
#define thread_context_init(ctx, thread, entry, arg) \
	(arch_kernel_call(thread_context_init, 0x0)(ctx, thread, entry, arg))

#define thread_context_type(ctx) \
	(arch_kernel_call(thread_context_type, CTX_UNKNOWN)(ctx))


#endif // ARCH_THREAD_H
