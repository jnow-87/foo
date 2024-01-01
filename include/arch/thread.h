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
#define thread_ctx_init(ctx, this_t, entry, arg) \
	(arch_kernel_call(thread_ctx_init, 0x0)(ctx, this_t, entry, arg))


#endif // ARCH_THREAD_H
