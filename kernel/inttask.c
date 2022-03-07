/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/interrupt.h>
#include <kernel/inttask.h>
#include <kernel/ksignal.h>
#include <kernel/memory.h>
#include <sys/list.h>
#include <sys/errno.h>
#include <sys/mutex.h>


/* global functions */
void itask_queue_init(itask_queue_t *queue){
	queue->head = 0x0;
	queue->tail = 0x0;
	mutex_init(&queue->mtx, MTX_NOINT);
}

int itask_issue(itask_queue_t *queue, void *data, int_num_t num){
	bool is_first;
	itask_t task;


	task.data = data;
	task.errno = E_OK;
	ksignal_init(&task.sig);

	mutex_lock(&queue->mtx);

	list1_add_tail(queue->head, queue->tail, &task);
	is_first = (list_first(queue->head) == &task);

	mutex_unlock(&queue->mtx);

	if(is_first)
		int_foretell(num);

	ksignal_wait(&task.sig);

	return_errno(task.errno);
}

void itask_complete(itask_queue_t *queue, errno_t e_code){
	itask_t *task;


	mutex_lock(&queue->mtx);

	task = list_first(queue->head);
	list1_rm_head(queue->head, queue->tail);
	task->errno = e_code;

	mutex_unlock(&queue->mtx);

	ksignal_send(&task->sig);
}

void *itask_query_data(itask_queue_t *queue){
	itask_t *task;


	mutex_lock(&queue->mtx);
	task = list_first(queue->head);
	mutex_unlock(&queue->mtx);

	return (task ? task->data : 0x0);
}
