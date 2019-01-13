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
#include <kernel/csection.h>
#include <sys/queue.h>
#include <sys/errno.h>


/* static variables */
static csection_lock_t sig_mtx = CSECTION_INITIALISER();


/* global functions */
void ksignal_init(ksignal_t *sig){
	sig->head = 0x0;
	sig->tail = 0x0;
}

/**
 * \pre		no kernel global mutexes, such as the scheduler
 * 			mutex must be locked, active locks on a dedicated
 * 			object such	as a file descriptor are fine
 */
int ksignal_wait(ksignal_t *sig){
	ksignal_queue_t *e;


	e = kmalloc(sizeof(ksignal_queue_t));

	if(e == 0x0)
		return_errno(E_NOMEM);

	e->thread = sched_running();

	csection_lock(&sig_mtx);

	queue_enqueue(sig->head, sig->tail, e);
	sched_thread_pause((thread_t*)e->thread);

	csection_unlock(&sig_mtx);

	sched_yield();

	return E_OK;
}

void ksignal_send(ksignal_t *sig){
	ksignal_queue_t *e;


	csection_lock(&sig_mtx);

	e = queue_dequeue(sig->head, sig->tail);

	if(e != 0x0){
		sched_thread_wake((thread_t*)e->thread);
		kfree(e);
	}

	csection_unlock(&sig_mtx);
}

void ksignal_bcast(ksignal_t *sig){
	ksignal_queue_t *e;


	csection_lock(&sig_mtx);

	while(1){
		e = queue_dequeue(sig->head, sig->tail);

		if(e == 0x0)
			break;

		sched_thread_wake((thread_t*)e->thread);
		kfree(e);
	}

	csection_unlock(&sig_mtx);
}
