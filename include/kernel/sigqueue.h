/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_SIGQUEUE_H
#define KERNEL_SIGQUEUE_H


#include <kernel/ksignal.h>
#include <sys/types.h>
#include <sys/mutex.h>


/* types */
typedef struct sigq_t{
	struct sigq_t *next;

	void *data;
	ksignal_t sig;
} sigq_t;

typedef struct{
	sigq_t *head,
		   *tail;

	mutex_t mtx;
} sigq_queue_t;


/* prototypes */
void sigq_queue_init(sigq_queue_t *queue);
sigq_t *sigq_create(void *data);
void sigq_init(sigq_t *e, void *data);

sigq_t *sigq_first(sigq_queue_t *queue);
bool sigq_enqueue(sigq_queue_t *queue, sigq_t *e);
void sigq_dequeue(sigq_queue_t *queue, sigq_t *e);

void sigq_wait(sigq_t *e);


#endif // KERNEL_SIGQUEUE_H
