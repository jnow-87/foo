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
#include <kernel/thread.h>
#include <sys/thread.h>
#include <sys/signal.h>


/* types */
typedef struct usignal_ctx_t{
	struct usignal_ctx_t *next;

	thread_ctx_t *ctx_addr;
	thread_ctx_t ctx;
} usignal_ctx_t;


/* prototypes */
int usignal_send(struct thread_t *this_t, signal_t sig);


#endif // KERNEL_USIGNAL_H
