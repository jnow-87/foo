/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/init.h>
#include <kernel/syscall.h>
#include <kernel/kprintf.h>
#include <kernel/panic.h>
#include <kernel/sched.h>
#include <kernel/process.h>
#include <kernel/thread.h>
#include <sys/string.h>
#include <sys/syscall.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/list.h>
#include <sys/stack.h>


/* local/static prototypes */
static int sc_hdlr_signal_register(void *p);
static int sc_hdlr_signal_send(void *p);
static int sc_hdlr_signal_return(void *p);


/* global functions */
int usignal_send(thread_t *this_t, signal_t num){
	usignal_t *sig;


	if(num >= SIG_MAX)
		return_errno(E_INVAL);

	DEBUG("signal %d: %s.%d to %s.%d\n",
		num,
		sched_running()->parent->name, sched_running()->tid,
		this_t->parent->name, this_t->tid
	);

	sig = kmalloc(sizeof(usignal_t));

	if(sig == 0x0){
		FATAL("out of memory, signal %d to %s.%d\n", num, this_t->parent->name, this_t->tid);
		return -errno;
	}

	sig->num = num;
	sig->pending = true;

	list_add_tail_safe(this_t->signals, sig, &this_t->mtx);

	return 0;
}

void usignal_destroy(struct thread_t *this_t){
	thread_ctx_t *ctx;
	usignal_t *sig;


	list_for_each(this_t->ctx_stack, ctx){
		if(ctx->type == CTX_SIGRETURN)
			kfree(ctx);
	}

	list_for_each(this_t->signals, sig)
		kfree(sig);
}

thread_ctx_t *usignal_entry(usignal_t *sig, thread_t *this_t, thread_ctx_t *ctx){
	thread_ctx_t *ret;


	ret = kmalloc(sizeof(thread_ctx_t));

	if(ret == 0x0)
		return ctx;

	memcpy(ret, ctx, sizeof(thread_ctx_t));
	ret->type = CTX_SIGRETURN;
	stack_push(this_t->ctx_stack, ret);

	thread_ctx_init(ctx, this_t, this_t->parent->sig_hdlr, (void*)sig->num);

	sig->pending = false;

	return ctx;
}

thread_ctx_t *usignal_return(usignal_t *sig, struct thread_t *this_t, thread_ctx_t *ctx){
	thread_ctx_t *ret;


	list_rm(this_t->signals, sig);
	kfree(sig);
	sig = list_first(this_t->signals);

	ret = ctx;
	ctx = ctx->this;

	if(sig == 0x0){
		memcpy(ctx, ret, sizeof(thread_ctx_t));
		ctx->type = CTX_USER;

		kfree(ret);
	}
	else{
		stack_push(this_t->ctx_stack, ret);
		thread_ctx_init(ctx, this_t, this_t->parent->sig_hdlr, (void*)sig->num);
		sig->pending = false;
	}

	return ctx;
}


/* local functions */
static int init(void){
	int r;


	r = 0;

	r |= sc_register(SC_SIGREGISTER, sc_hdlr_signal_register);
	r |= sc_register(SC_SIGSEND, sc_hdlr_signal_send);
	r |= sc_register(SC_SIGRETURN, sc_hdlr_signal_return);

	return r;
}

kernel_init(0, init);

static int sc_hdlr_signal_register(void *_p){
	sc_signal_t *p;
	process_t *this_p;


	p = (sc_signal_t*)_p;

	this_p = process_find(p->pid);

	if(this_p == 0x0)
		return_errno(E_INVAL);

	DEBUG("%s: %p\n", this_p->name, p->hdlr);
	this_p->sig_hdlr = p->hdlr;

	return 0;
}

static int sc_hdlr_signal_send(void *_p){
	sc_signal_t *p;
	process_t *this_p;
	thread_t *this_t;


	p = (sc_signal_t*)_p;

	/* get target thread */
	this_p = process_find(p->pid);

	if(this_p == 0x0)
		return_errno(E_INVAL);

	this_t = list_find_safe(this_p->threads, tid, p->tid, &this_p->mtx);

	if(this_t == 0x0)
		return_errno(E_INVAL);

	/* trigger signal */
	return usignal_send(this_t, p->sig);
}

static int sc_hdlr_signal_return(void *_p){
	thread_t *this_t;


	this_t = (thread_t*)sched_running();

	DEBUG("%s.%d\n", this_t->parent->name, this_t->tid);

	mutex_lock(&this_t->mtx);
	stack_pop(this_t->ctx_stack);
	mutex_unlock(&this_t->mtx);

	return 0;
}
