/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#include <kernel/sigqueue.h>
#include <kernel/ksignal.h>
#include <kernel/memory.h>
#include <sys/mutex.h>
#include <sys/list.h>



/* global functions */
void sigq_queue_init(sigq_queue_t *queue){
	queue->head = 0x0;
	queue->tail = 0x0;
	mutex_init(&queue->mtx, 0);
}

sigq_t *sigq_create(void *data){
	sigq_t *e;


	e = kmalloc(sizeof(sigq_t));

	if(e == 0x0)
		return 0x0;

	sigq_init(e, data);

	return e;
}

void sigq_init(sigq_t *e, void *data){
	e->data = data;
	ksignal_init(&e->sig);
}

sigq_t *sigq_first(sigq_queue_t *queue){
	return list_first_safe(queue->head, &queue->mtx);
}

bool sigq_enqueue(sigq_queue_t *queue, sigq_t *e){
	bool is_ready;


	mutex_lock(&queue->mtx);

	is_ready = list_empty(queue->head);
	list1_add_tail(queue->head, queue->tail, e);

	mutex_unlock(&queue->mtx);

	return is_ready;
}

void sigq_dequeue(sigq_queue_t *queue, sigq_t *e){
	mutex_lock(&queue->mtx);
	list1_rm_head(queue->head, queue->tail);
	mutex_unlock(&queue->mtx);

	ksignal_send(&e->sig);
}

void sigq_wait(sigq_t *e){
	ksignal_wait(&e->sig);
}
