#include <arch/interrupt.h>
#include <kernel/signal.h>
#include <kernel/sched.h>
#include <kernel/memory.h>
#include <sys/list.h>
#include <sys/mutex.h>
#include <sys/errno.h>


/* static varaibles */
static mutex_t sig_mtx = MUTEX_INITIALISER();


/* global functions */
void ksignal_init(ksignal_t *sig){
	*sig = 0x0;
}

int ksignal_wait(ksignal_t *sig){
	int_type_t imask;
	ksignal_el_t *e;


	e = kmalloc(sizeof(ksignal_el_t));

	if(e == 0x0)
		return_errno(E_NOMEM);

	e->thread = sched_running();

	imask = int_enable(INT_NONE);
	mutex_lock(&sig_mtx);

	list_add_tail(*sig, e);
	sched_pause();

	mutex_unlock(&sig_mtx);
	int_enable(imask);

	sched_yield();

	return E_OK;
}

void ksignal_send(ksignal_t *sig){
	int_type_t imask;
	ksignal_el_t *e;


	imask = int_enable(INT_NONE);
	mutex_lock(&sig_mtx);

	e = list_first(*sig);

	if(e != 0x0){
		list_rm(*sig, e);
		sched_wake((thread_t*)e->thread);
		kfree(e);
	}

	mutex_unlock(&sig_mtx);
	int_enable(imask);
}

void ksignal_bcast(ksignal_t *sig){
	int_type_t imask;
	ksignal_el_t *e;


	imask = int_enable(INT_NONE);
	mutex_lock(&sig_mtx);

	list_for_each(*sig, e){
		list_rm(*sig, e);
		sched_wake((thread_t*)e->thread);
		kfree(e);
	}

	mutex_unlock(&sig_mtx);
	int_enable(imask);
}
