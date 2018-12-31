/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_THREAD_H
#define ARCH_THREAD_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define thread_context_init(ctx, thread, user_entry, thread_entry, thread_arg) \
	(arch_kernel_call(thread_context_init, 0x0)(ctx, thread, user_entry, thread_entry, thread_arg))


#endif // ARCH_THREAD_H
