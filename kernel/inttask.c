/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#include <arch/interrupt.h>
#include <kernel/critsec.h>
#include <kernel/inttask.h>
#include <kernel/ksignal.h>
#include <kernel/memory.h>
#include <sys/list.h>
#include <sys/errno.h>


/* global functions */
void itask_queue_init(itask_queue_t *queue){
	queue->head = 0x0;
	queue->tail = 0x0;
	critsec_init(&queue->lock);
}

void itask_issue(itask_queue_t *queue, void *data, int_num_t num){
	bool is_first;
	itask_t task;


	task.data = data;
	ksignal_init(&task.sig);

	critsec_lock(&queue->lock);

	list1_add_tail(queue->head, queue->tail, &task);
	is_first = (list_first(queue->head) == &task);

	critsec_unlock(&queue->lock);

	if(is_first)
		int_call(num);

	ksignal_wait(&task.sig);
}

void itask_complete(itask_queue_t *queue){
	itask_t *task;


	critsec_lock(&queue->lock);

	task = list_first(queue->head);
	list1_rm_head(queue->head, queue->tail);
	task->errno = e_code;

	critsec_unlock(&queue->lock);

	ksignal_send(&task->sig);
}

void *itask_query_data(itask_queue_t *queue){
	itask_t *task;


	critsec_lock(&queue->lock);
	task = list_first(queue->head);
	critsec_unlock(&queue->lock);

	return (task ? task->data : 0x0);
}
