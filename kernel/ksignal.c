/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/interrupt.h>
#include <kernel/ksignal.h>
#include <kernel/timer.h>
#include <kernel/sched.h>
#include <kernel/memory.h>
#include <sys/list.h>
#include <sys/mutex.h>
#include <sys/errno.h>


/* types */
typedef struct{
	ksignal_t *sig;
	mutex_t *mtx;
} timeout_data_t;


/* local/static prototypes */
static void timeout_hdlr(void *payload);


/* global functions */
void ksignal_init(ksignal_t *sig){
	*sig = 0x0;
}

void ksignal_wait(ksignal_t *sig, mutex_t *mtx){
	_ksignal_t e;


	e.thread = sched_running();

	list_add_tail(*sig, &e);
	sched_thread_pause((thread_t*)e.thread);

	mutex_unlock(mtx);
	sched_yield();
	mutex_lock(mtx);
}

int ksignal_timedwait(ksignal_t *sig, mutex_t *mtx, uint32_t timeout_us){
	ktimer_t timer;
	timeout_data_t to;


	to.sig = sig;
	to.mtx = mtx;

	if(!(mtx->attr & MTX_NOINT))
		return_errno(E_INVAL);

	ktimer_register(&timer, timeout_us, timeout_hdlr, &to, false);
	ksignal_wait(sig, mtx);
	ktimer_release(&timer);

	return 0;
}

void ksignal_send(ksignal_t *sig){
	_ksignal_t *e;


	e = list_first(*sig);

	if(e == 0x0)
		return;

	sched_thread_wake((thread_t*)e->thread);
	list_rm(*sig, e);
}

void ksignal_bcast(ksignal_t *sig){
	while(list_first(*sig)){
		ksignal_send(sig);
	}
}


/* local functions */
static void timeout_hdlr(void *payload){
	timeout_data_t *p = (timeout_data_t*)payload;


	mutex_lock(p->mtx);
	ksignal_send(p->sig);
	mutex_unlock(p->mtx);
}
