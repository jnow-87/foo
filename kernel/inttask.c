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
#include <sys/queue.h>
#include <sys/errno.h>
#include <sys/mutex.h>


/* global functions */
void itask_queue_init(itask_queue_t *queue){
	queue->head = 0x0;
	queue->tail = 0x0;
	mutex_init(&queue->mtx, MTX_NOINT);
}

void itask_queue_destroy(itask_queue_t *queue){
	while(itask_payload(queue, 0x0)){
		itask_complete(queue, E_END);
	}
}

int itask_issue(itask_queue_t *queue, void *payload, int_num_t num){
	bool is_first;
	itask_t task;


	task.payload = payload;
	task.errnum = 0;
	ksignal_init(&task.sig);

	mutex_lock(&queue->mtx);

	queue_enqueue(queue->head, queue->tail, &task);
	is_first = (list_first(queue->head) == &task);

	if(is_first)
		int_foretell(num);

	ksignal_wait(&task.sig, &queue->mtx);
	mutex_unlock(&queue->mtx);

	return_errno(task.errnum);
}

void itask_complete(itask_queue_t *queue, errno_t errnum){
	itask_t *task;


	mutex_lock(&queue->mtx);

	task = queue_dequeue(queue->head, queue->tail);

	if(task != 0x0){
		task->errnum = errnum;
		ksignal_send(&task->sig);
	}

	mutex_unlock(&queue->mtx);
}

void *itask_payload(itask_queue_t *queue, int (*complete)(void *payload)){
	int e;
	itask_t *task;


	while(1){
		mutex_lock(&queue->mtx);
		task = list_first(queue->head);
		mutex_unlock(&queue->mtx);

		if(task == 0x0)
			return 0x0;

		e = (complete == 0x0) ? -1 : complete(task->payload);

		if(e < 0)
			return task->payload;

		itask_complete(queue, e);
	}
}
