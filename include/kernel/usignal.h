/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_USIGNAL_H
#define KERNEL_USIGNAL_H


#include <arch/thread.h>
#include <kernel/thread.h>
#include <sys/signal.h>


/* types */
typedef struct usignal_t{
	struct usignal_t *prev,
					 *next;

	signal_t num;
	bool pending;
} usignal_t;


/* prototypes */
int usignal_send(struct thread_t *this_t, signal_t num);
void usignal_destroy(struct thread_t *this_t);

thread_ctx_t *usignal_entry(usignal_t *sig, struct thread_t *this_t, thread_ctx_t *ctx);
thread_ctx_t *usignal_return(usignal_t *sig, struct thread_t *this_t, thread_ctx_t *ctx);


#endif // KERNEL_USIGNAL_H
