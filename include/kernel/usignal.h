/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_USIGNAL_H
#define KERNEL_USIGNAL_H


#include <arch/thread.h>
#include <kernel/memory.h>


/* types */
typedef struct usignal_ctx_t{
	struct usignal_ctx_t *next;

	thread_context_t *ctx_addr;
	thread_context_t ctx;
} usignal_ctx_t;


#endif // KERNEL_USIGNAL_H
