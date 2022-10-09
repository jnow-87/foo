/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_INTTASK_H
#define KERNEL_INTTASK_H


#include <kernel/interrupt.h>
#include <kernel/ksignal.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mutex.h>


/* types */
typedef struct itask_t{
	struct itask_t *next;

	void *payload;
	errno_t errno;

	ksignal_t sig;
} itask_t;

typedef struct{
	itask_t *head,
			*tail;

	mutex_t mtx;
} itask_queue_t;


/* prototypes i*/
void itask_queue_init(itask_queue_t *queue);
void itask_queue_destroy(itask_queue_t *queue);

int itask_issue(itask_queue_t *queue, void *payload, int_num_t num);
void itask_complete(itask_queue_t *queue, errno_t ecode);
void *itask_query_payload(itask_queue_t *queue, int (*complete)(void *payload));


#endif // KERNEL_INTTASK_H
