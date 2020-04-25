/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/core.h>
#include <arch/memory.h>
#include <kernel/init.h>
#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <kernel/kprintf.h>
#include <kernel/usignal.h>
#include <sys/errno.h>
#include <sys/syscall.h>
#include <sys/list.h>


/* static/local prototypes */
static int sc_hdlr_sched_yield(void *p);

static int sc_hdlr_process_create(void *p);
static int sc_hdlr_process_info(void *p);

static int sc_hdlr_thread_create(void *p);
static int sc_hdlr_thread_info(void *p);
static int sc_hdlr_nice(void *p);
static int sc_hdlr_exit(void *p);


/* local functions */
static int init(void){
	int r;


	/* register syscalls */
	r = E_OK;

	r |= sc_register(SC_SCHEDYIELD, sc_hdlr_sched_yield);

	r |= sc_register(SC_PROCCREATE, sc_hdlr_process_create);
	r |= sc_register(SC_PROCINFO, sc_hdlr_process_info);

	r |= sc_register(SC_EXIT, sc_hdlr_exit);
	r |= sc_register(SC_THREADCREATE, sc_hdlr_thread_create);
	r |= sc_register(SC_THREADINFO, sc_hdlr_thread_info);
	r |= sc_register(SC_NICE, sc_hdlr_nice);

	return r;
}

kernel_init(2, init);

static int sc_hdlr_sched_yield(void *p){
	sched_trigger();
	return E_OK;
}

static int sc_hdlr_process_create(void *_p){
	sc_process_t *p = (sc_process_t*)_p;
	char name[p->name_len + 1];
	char args[p->args_len + 1];
	process_t *this_p,
			  *new;


	p->pid = 0;

	/* process arguments */
	this_p = sched_running()->parent;

	copy_from_user(name, p->name, p->name_len + 1, this_p);
	copy_from_user(args, p->args, p->args_len + 1, this_p);

	/* create process */
	DEBUG("create process \"%s\" with args \"%s\"\n", name, args);

	new = process_create(p->binary, p->bin_type, p->name, p->args, this_p->cwd);

	if(new == 0x0)
		return -errno;

	sched_thread_wake(list_first(new->threads));

	p->pid = new->pid;

	return E_OK;
}

static int sc_hdlr_process_info(void *_p){
	sc_process_t *p;
	process_t *this_p;


	p = (sc_process_t*)_p;
	this_p = sched_running()->parent;

	p->pid = this_p->pid;

	return E_OK;
}

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

	sched_thread_wake(new);

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

static int sc_hdlr_exit(void *_p){
	sc_exit_t *p;
	thread_t *this_t,
			 *sibl;


	p = (sc_exit_t*)_p;
	this_t = (thread_t*)sched_running();

	DEBUG("%s.%u exit with status %d\n", this_t->parent->name, this_t->tid, p->status);

	/* exit siblings */
	if(this_t->tid == 0 || p->kill_siblings){
		mutex_lock(&this_t->parent->mtx);

		list_for_each(this_t->parent->threads, sibl){
			DEBUG("kill %s.%u\n", sibl->parent->name, sibl->tid);
			usignal_send(sibl, SIG_KILL);
		}

		mutex_unlock(&this_t->parent->mtx);
	}

	/* exit current thread */
	// ensure thread is no longer the running one
	sched_thread_bury(this_t);
	sched_trigger();

	return E_OK;
}
