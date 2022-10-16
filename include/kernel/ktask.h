/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H


#include <config/config.h>
#include <kernel/ktaskqueue.h>
#include <sys/types.h>


/* types */
typedef enum{
	TASK_RECURRING = 0x1,
	TASK_READY = 0x2,
} ktask_flag_t;

typedef void (*ktask_hdlr_t)(void *payload);

typedef struct ktask_t{
	struct ktask_t *prev,
				   *next;

#ifdef CONFIG_KERNEL_KTASK_QUEUE
	ktask_queue_t *dep_queue;
#endif // CONFIG_KERNEL_KTASK_QUEUE

	ktask_flag_t flags;

	ktask_hdlr_t hdlr;
	uint8_t payload[];
} ktask_t;


/* prototypes */
int ktask_create(ktask_hdlr_t hdlr, void *payload, size_t size, ktask_queue_t *dep_queue, bool recurring);

void ktask_complete(ktask_t *task);
void ktask_cancel(ktask_t *task);

ktask_t *ktask_next(void);


#endif // KERNEL_TASK_H
