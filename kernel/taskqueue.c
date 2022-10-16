/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/memory.h>
#include <kernel/ktask.h>
#include <kernel/ktaskqueue.h>
#include <kernel/sched.h>
#include <sys/errno.h>
#include <sys/queue.h>


/* global functions */
/**
 * \brief	create a task dependency queue
 *
 * \return	pointer to the queue
 * 			0x0 in case of an error, with errno being set appropriately
 * 				E_NOMEM		not enough memory to allocate the queue
 */
ktask_queue_t *ktask_queue_create(void){
	ktask_queue_t *queue;


	queue = kmalloc(sizeof(ktask_queue_t));

	if(queue == 0x0)
		return 0x0;

	queue->head = 0x0;
	queue->tail = 0x0;

	return queue;
}

/**
 * \brief	destroy the given task dependency queue
 *
 * \param	queue	queue to destroy
 */
void ktask_queue_destroy(ktask_queue_t *queue){
	ktask_queue_entry_t *e;


	while(!queue_empty(queue->head)){
		e = queue_head(queue->head);

		ktask_cancel(e->task);
		queue_dequeue(queue->head, queue->tail);
		kfree(e);
	}

	kfree(queue);
}

/**
 * \brief	flush the given dependency queue, i.e. wait for it's tasks
 * 			to be processed
 *
 * \param	queue	queue to flush
 */
void ktask_queue_flush(ktask_queue_t *queue){
	while(!queue_empty(queue->head))
		sched_yield();
}

/**
 * \brief	add a task to a dependency queue
 *
 * \param	queue	queue to add to
 * \param	task	task to add
 *
 * \return	0 on success, negative value on error
 */
int ktask_queue_enqueue(ktask_queue_t *queue, struct ktask_t *task){
	ktask_queue_entry_t *e;


	e = kmalloc(sizeof(ktask_queue_entry_t));

	if(e == 0x0)
		return -errno;

	if(!queue_empty(queue->head))
		task->flags &= ~TASK_READY;

	e->task = task;
	queue_enqueue(queue->head, queue->tail, e);

	return 0;
}

/**
 * \brief	remove a task from a dependency queue
 *
 * \param	queue	queue to remove from
 * \param	task	task to remove
 */
void ktask_queue_dequeue(ktask_queue_t *queue){
	ktask_queue_entry_t *e;


	e = queue_dequeue(queue->head, queue->tail);

	if(e->task->flags & TASK_RECURRING){
		e->task->flags &= ~TASK_READY;
		queue_enqueue(queue->head, queue->tail, e);
	}
	else
		kfree(e);

	e = queue_head(queue->head);

	if(e != 0x0)
		e->task->flags |= TASK_READY;
}
