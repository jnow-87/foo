/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_INTTASK_H
#define KERNEL_INTTASK_H


#include <arch/interrupt.h>
#include <kernel/ksignal.h>
#include <kernel/critsec.h>
#include <sys/types.h>


/* types */
typedef struct itask_t{
	struct itask_t *next;

	void *data;
	ksignal_t sig;
} itask_t;

typedef struct{
	itask_t *head,
			*tail;

	critsec_lock_t lock;
} itask_queue_t;


/* prototypes i*/
void itask_queue_init(itask_queue_t *queue);

void itask_issue(itask_queue_t *queue, void *data, int_num_t num);
void itask_complete(itask_queue_t *queue);
void *itask_query_data(itask_queue_t *queue);


#endif // KERNEL_INTTASK_H
