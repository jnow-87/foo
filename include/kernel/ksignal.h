/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_SIGNAL_H
#define KERNEL_SIGNAL_H


#include <sys/mutex.h>
#include <sys/types.h>


/* incomplete types */
struct kthread_t;


/* types */
typedef struct _ksignal_t{
	struct _ksignal_t *prev,
					  *next;

	struct thread_t *thread;
} _ksignal_t;

typedef _ksignal_t *ksignal_t;


/* prototypes */
void ksignal_init(ksignal_t *sig);

void ksignal_wait(ksignal_t *sig, mutex_t *mtx);
int ksignal_timedwait(ksignal_t *sig, mutex_t *mtx, uint32_t timeout_us);

void ksignal_send(ksignal_t *sig);
void ksignal_bcast(ksignal_t *sig);


#endif // KERNEL_SIGNAL_H
