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

	void *data;
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

int itask_issue(itask_queue_t *queue, void *data, int_num_t num);
void itask_complete(itask_queue_t *queue, errno_t e_code);
void *itask_query_data(itask_queue_t *queue);


#endif // KERNEL_INTTASK_H
