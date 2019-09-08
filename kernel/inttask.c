/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#include <arch/interrupt.h>
#include <kernel/inttask.h>
#include <kernel/ksignal.h>
#include <kernel/memory.h>
#include <sys/mutex.h>
#include <sys/list.h>
#include <sys/errno.h>


/* global functions */
void itask_queue_init(itask_queue_t *queue){
	queue->head = 0x0;
	queue->tail = 0x0;
	mutex_init(&queue->mtx, 0);
}

void itask_issue(itask_queue_t *queue, void *data, int_num_t num){
	bool empty;
	itask_t e;


	e.data = data;
	ksignal_init(&e.sig);

	mutex_lock(&queue->mtx);

	empty = list_empty(queue->head);
	list1_add_tail(queue->head, queue->tail, &e);

	mutex_unlock(&queue->mtx);

	if(empty)
		int_call(num);

	ksignal_wait(&e.sig);
}

void itask_complete(itask_queue_t *queue){
	itask_t *e;


	mutex_lock(&queue->mtx);

	e = list_first(queue->head);
	list1_rm_head(queue->head, queue->tail);

	mutex_unlock(&queue->mtx);

	ksignal_send(&e->sig);
}

void *itask_query_data(itask_queue_t *queue){
	itask_t *e;


	e = list_first_safe(queue->head, &queue->mtx);

	return (e ? e->data : 0x0);
}
