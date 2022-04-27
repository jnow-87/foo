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

	if(is_first)
		int_foretell(num);

	ksignal_wait(&task.sig, &queue->mtx);
	mutex_unlock(&queue->mtx);

	return_errno(task.errno);
}

void itask_complete(itask_queue_t *queue, errno_t ecode){
	itask_t *task;


	mutex_lock(&queue->mtx);

	task = list_first(queue->head);
	list1_rm_head(queue->head, queue->tail);
	task->errno = ecode;

	ksignal_send(&task->sig);

	mutex_unlock(&queue->mtx);
}

void *itask_query_data(itask_queue_t *queue, int (*complete)(void *data)){
	int ecode;
	itask_t *task;


	while(1){
		mutex_lock(&queue->mtx);
		task = list_first(queue->head);
		mutex_unlock(&queue->mtx);

		if(task == 0x0)
			return 0x0;

		ecode = (complete == 0x0) ? -1 : complete(task->data);

		if(ecode < 0)
			return task->data;

		itask_complete(queue, ecode);
	}
}
