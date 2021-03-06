/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_SIGNAL_H
#define KERNEL_SIGNAL_H


#include <kernel/critsec.h>
#include <sys/mutex.h>
#include <sys/types.h>


/* incomplete types */
struct kthread_t;


/* types */
typedef struct ksignal_queue_t{
	struct ksignal_queue_t *next;

	struct thread_t const *thread;
} ksignal_queue_t;

typedef struct{
	ksignal_queue_t *head,
					*tail;

	bool interim;
} ksignal_t;


/* prototypes */
void ksignal_init(ksignal_t *sig);

void ksignal_wait(ksignal_t *sig);
void ksignal_wait_mtx(ksignal_t *sig, mutex_t *mtx);
void ksignal_wait_crit(ksignal_t *sig, critsec_lock_t *lock);

void ksignal_send(ksignal_t *sig);
void ksignal_bcast(ksignal_t *sig);


#endif // KERNEL_SIGNAL_H
