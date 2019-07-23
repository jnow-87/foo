/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_SIGNAL_H
#define KERNEL_SIGNAL_H


#include <sys/types.h>


/* incomplete types */
struct kthread_t;


/* types */
typedef struct ksignal_queue_t{
	struct ksignal_queue_t *next;

	struct thread_t const *thread;
} ksignal_queue_t;

typedef struct{
	uint8_t unmatched;

	ksignal_queue_t *head,
					*tail;
} ksignal_t;


/* prototypes */
void ksignal_init(ksignal_t *sig);

void ksignal_wait(ksignal_t *sig);
void ksignal_send(ksignal_t *sig);
void ksignal_bcast(ksignal_t *sig);


#endif // KERNEL_SIGNAL_H
