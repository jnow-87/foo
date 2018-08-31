#include <kernel/init.h>
#include <kernel/syscall.h>
#include <kernel/kprintf.h>
#include <kernel/sched.h>
#include <kernel/panic.h>
#include <kernel/csection.h>
#include <sys/errno.h>
#include <sys/syscall.h>
#include <sys/list.h>
#include "sched.h"


/* local/static prototypes */
static int sc_hdlr_thread_create(void *p);
static int sc_hdlr_thread_info(void *p);
static int sc_hdlr_nice(void *p);
static int sc_hdlr_exit(void *p);


/* local functions */
static int init(void){
	int r;


	/* register syscalls */
	r = E_OK;

	r |= sc_register(SC_EXIT, sc_hdlr_exit);
	r |= sc_register(SC_THREADCREATE, sc_hdlr_thread_create);
	r |= sc_register(SC_THREADINFO, sc_hdlr_thread_info);
	r |= sc_register(SC_NICE, sc_hdlr_nice);

	return r;
}

kernel_init(2, init);

static int sc_hdlr_thread_create(void *_p){
	sc_thread_t *p;
	thread_t *new;
	process_t *this_p;


	p = (sc_thread_t*)_p;
	this_p = sched_running()->parent;

	p->tid = 0;

	DEBUG("create thread for \"%s\" at %p, arg %p\n", this_p->name, p->entry, p->arg);

	mutex_lock(&this_p->mtx);
	new = thread_create(this_p, list_last(this_p->threads)->tid + 1, p->entry, p->arg);
	mutex_unlock(&this_p->mtx);

	if(new == 0x0)
		return -errno;

	csection_lock(&sched_lock);
	sched_transition(new, READY);
	csection_unlock(&sched_lock);

	p->tid = new->tid;

	return E_OK;
}

static int sc_hdlr_thread_info(void *_p){
	sc_thread_t *p;
	thread_t const *this_t;


	p = (sc_thread_t*)_p;
	this_t = sched_running();

	p->tid = this_t->tid;
	p->priority = this_t->priority;
	p->affinity = this_t->affinity;

	return E_OK;
}

static int sc_hdlr_nice(void *_p){
	sc_thread_t *p;
	thread_t const *this_t;


	p = (sc_thread_t*)_p;
	this_t = sched_running();

	((thread_t*)this_t)->priority += p->priority;

	p->priority = this_t->priority;

	return E_OK;
}

static int sc_hdlr_exit(void *p){
	process_t *this_p;
	thread_t const *this_t;


	this_t = sched_running();
	this_p = this_t->parent;

	DEBUG("thread %s.%u exit with status %d\n", this_p->name, this_t->tid, *((int*)p));

	/* ensure thread is no longer the running one */
	sched_tick();
	sched_transition((thread_t*)this_t, DEAD);

	return E_OK;
}
