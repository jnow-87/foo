/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_TASK_QUEUE_H
#define KERNEL_TASK_QUEUE_H


#include <kernel/ktask.h>
#include <sys/types.h>


/* incomplete types */
struct ktask_t;


/* types */
typedef struct ktask_queue_entry_t{
	struct ktask_queue_entry_t *next;
	struct ktask_t *task;
} ktask_queue_entry_t;

typedef struct ktask_queue_t{
	ktask_queue_entry_t * volatile head,
						* volatile tail;
} ktask_queue_t;


/* prototypes */
ktask_queue_t *ktask_queue_create(void);
void ktask_queue_destroy(ktask_queue_t *queue);
void ktask_queue_flush(ktask_queue_t *queue);

int ktask_queue_enqueue(ktask_queue_t *queue, struct ktask_t *task);
void task_queue_dequeue(ktask_queue_t *queue);


#endif // KERNEL_TASK_QUEUE_H
