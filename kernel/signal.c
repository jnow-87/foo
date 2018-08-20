#include <arch/interrupt.h>
#include <kernel/signal.h>
#include <kernel/sched.h>
#include <kernel/memory.h>
#include <sys/queue.h>
#include <sys/mutex.h>
#include <sys/errno.h>


/* static varaibles */
static mutex_t sig_mtx = MUTEX_INITIALISER();


/* global functions */
void ksignal_init(ksignal_t *sig){
	sig->head = 0x0;
	sig->tail = 0x0;
}

int ksignal_wait(ksignal_t *sig){
	int_type_t imask;
	ksignal_queue_t *e;


	e = kmalloc(sizeof(ksignal_queue_t));

	if(e == 0x0)
		return_errno(E_NOMEM);

	e->thread = sched_running();

	imask = int_enable(INT_NONE);
	mutex_lock(&sig_mtx);

	queue_enqueue(sig->head, sig->tail, e);
	sched_pause();

	mutex_unlock(&sig_mtx);
	int_enable(imask);

	sched_yield();

	return E_OK;
}

void ksignal_send(ksignal_t *sig){
	int_type_t imask;
	ksignal_queue_t *e;


	imask = int_enable(INT_NONE);
	mutex_lock(&sig_mtx);

	e = queue_dequeue(sig->head, sig->tail);

	if(e != 0x0){
		sched_wake((thread_t*)e->thread);
		kfree(e);
	}

	mutex_unlock(&sig_mtx);
	int_enable(imask);
}

void ksignal_bcast(ksignal_t *sig){
	int_type_t imask;
	ksignal_queue_t *e;


	imask = int_enable(INT_NONE);
	mutex_lock(&sig_mtx);

	while(1){
		e = queue_dequeue(sig->head, sig->tail);

		if(e == 0x0)
			break;

		sched_wake((thread_t*)e->thread);
		kfree(e);
	}

	mutex_unlock(&sig_mtx);
	int_enable(imask);
}
