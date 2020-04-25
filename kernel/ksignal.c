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
#include <kernel/critsec.h>
#include <sys/queue.h>
#include <sys/errno.h>


/* static variables */
static critsec_lock_t ksignal_lock = CRITSEC_INITIALISER();


/* global functions */
void ksignal_init(ksignal_t *sig){
	sig->unmatched = 0;
	sig->head = 0x0;
	sig->tail = 0x0;
}

/**
 * \pre		no kernel global mutexes, such as the scheduler
 * 			mutex must be locked, active locks on a dedicated
 * 			object such	as a file descriptor are fine
 */
void ksignal_wait(ksignal_t *sig){
	ksignal_queue_t e;


	e.thread = sched_running();

	critsec_lock(&ksignal_lock);

	if(sig->unmatched){
		sig->unmatched--;
		csection_unlock(&sig_mtx);

		return;
	}

	queue_enqueue(sig->head, sig->tail, &e);
	sched_thread_pause((thread_t*)e.thread);

	critsec_unlock(&sig_mtx);

	sched_yield();
}

void ksignal_send(ksignal_t *sig){
	ksignal_queue_t *e;


	critsec_lock(&ksignal_lock);

	e = queue_dequeue(sig->head, sig->tail);

	if(e != 0x0)	sched_thread_wake((thread_t*)e->thread);
	else			sig->unmatched++;

	critsec_unlock(&ksignal_lock);
}

void ksignal_bcast(ksignal_t *sig){
	ksignal_queue_t *e;


	critsec_lock(&ksignal_lock);

	if(list_empty(sig->head))
		sig->unmatched++;

	while(1){
		e = queue_dequeue(sig->head, sig->tail);

		if(e == 0x0)
			break;

		sched_thread_wake((thread_t*)e->thread);
	}

	critsec_unlock(&ksignal_lock);
}
