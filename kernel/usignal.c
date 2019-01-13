/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/syscall.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/list.h>
#include <sys/stack.h>
#include <kernel/init.h>
#include <kernel/syscall.h>
#include <kernel/kprintf.h>
#include <kernel/panic.h>
#include <kernel/sched.h>
#include <kernel/process.h>
#include <kernel/thread.h>


/* local/static prototypes */
static int sc_hdlr_signal_register(void *p);
static int sc_hdlr_signal_send(void *p);
static int sc_hdlr_signal_return(void *p);

static void context_inject(thread_t *this_t, void *p);
static void context_extract(thread_t *this_t, void *p);


/* global functions */
int usignal_send(struct thread_t *this_t, signal_t sig){
	sc_signal_t p;


	p.sig = sig;

	DEBUG("%s.%d: to %s.%d, signal %d\n", sched_running()->parent->name, sched_running()->tid, this_t->parent->name, this_t->tid, sig);
	sched_thread_modify(this_t, context_inject, &p, sizeof(sc_signal_t));

	return -errno;
}


/* local functions */
static int init(void){
	int r;


	r = E_OK;

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

	return E_OK;
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
	sched_thread_modify(this_t, context_extract, 0x0, 0);

	return E_OK;
}

static void context_inject(thread_t *this_t, void *_p){
	sc_signal_t *p;
	usignal_ctx_t *sig_ctx;
	thread_ctx_t *ctx;


	p = (sc_signal_t*)_p;

	/* get first non-kernel context */
	list_for_each(this_t->ctx_stack, ctx){
		if(thread_context_type(ctx) == CTX_USER)
			break;
	}

	if(ctx == 0x0)
		kpanic(this_t, "no user-context found\n");

	/* save ctx to signal_ctx_stack */
	sig_ctx = kmalloc(sizeof(usignal_ctx_t));

	if(sig_ctx == 0x0){
		FATAL("out of memory, sending signal %d to %s.%d\n", p->sig, this_t->parent->name, this_t->tid);
		return;
	}

	sig_ctx->ctx_addr = ctx;
	memcpy(&sig_ctx->ctx, ctx, sizeof(thread_ctx_t));
	stack_push(this_t->signal_ctx_stack, sig_ctx);

	/* inject signal handler */
	thread_context_init(ctx, this_t, this_t->parent->sig_hdlr, 0x0, (void*)p->sig);
	ctx->next = sig_ctx->ctx.next;
}

static void context_extract(thread_t *this_t, void *p){
	thread_ctx_t *ctx;
	usignal_ctx_t *sig_ctx;


	/* restore origianl context from signal_ctx_stack */
	sig_ctx = stack_pop(this_t->signal_ctx_stack);

	// use origianl context address since the current context, with all its stack allocations
	// needs to be dropped, i.e. all local variables used by the signal hdlr
	ctx = sig_ctx->ctx_addr;

	memcpy(ctx, &sig_ctx->ctx, sizeof(thread_ctx_t));
	kfree(sig_ctx);

	(void)stack_pop(this_t->ctx_stack);
	stack_push(this_t->ctx_stack, ctx);
}
