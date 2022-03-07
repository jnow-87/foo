/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/interrupt.h>
#include <kernel/ksignal.h>
#include <kernel/sched.h>
#include <kernel/memory.h>
#include <sys/queue.h>
#include <sys/mutex.h>
#include <sys/errno.h>


/* static variables */
static mutex_t ksignal_mtx = NOINT_MUTEX_INITIALISER();


/* global functions */
void ksignal_init(ksignal_t *sig){
	sig->head = 0x0;
	sig->tail = 0x0;
	sig->interim = false;
}

void ksignal_wait(ksignal_t *sig){
	ksignal_queue_t e;


	e.thread = sched_running();

	mutex_lock(&ksignal_mtx);

	if(!sig->interim){
		queue_enqueue(sig->head, sig->tail, &e);
		sched_thread_pause((thread_t*)e.thread);

		mutex_unlock(&ksignal_mtx);

		sched_yield();
	}
	else{
		sig->interim = false;
		mutex_unlock(&ksignal_mtx);
	}
}

void ksignal_wait_mtx(ksignal_t *sig, mutex_t *mtx){
	mutex_unlock(mtx);
	ksignal_wait(sig);
	mutex_lock(mtx);
}

void ksignal_send(ksignal_t *sig){
	ksignal_queue_t *e;


	mutex_lock(&ksignal_mtx);

	e = queue_dequeue(sig->head, sig->tail);

	if(e != 0x0)	sched_thread_wake((thread_t*)e->thread);
	else			sig->interim = true;

	mutex_unlock(&ksignal_mtx);
}

void ksignal_bcast(ksignal_t *sig){
	ksignal_queue_t *e;


	mutex_lock(&ksignal_mtx);

	if(list_empty(sig->head))
		sig->interim = true;

	while(1){
		e = queue_dequeue(sig->head, sig->tail);

		if(e == 0x0)
			break;

		sched_thread_wake((thread_t*)e->thread);
	}

	mutex_unlock(&ksignal_mtx);
}
