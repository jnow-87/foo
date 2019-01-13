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
#include <kernel/sched.h>
#include <kernel/process.h>
#include <kernel/thread.h>
#include <kernel/ipi.h>


/* local/static prototypes */
static int sc_hdlr_signal_register(void *p);
static int sc_hdlr_signal_send(void *p);
static int sc_hdlr_signal_return(void *p);

#ifdef CONFIG_KERNEL_SMP
static void ipi_hdlr(void *data);
#endif // CONFIG_KERNEL_SMP

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
	usignal_ctx_t *sig_ctx;
	thread_context_t *ctx;
#ifdef CONFIG_KERNEL_SMP
	int core;
#endif // CONFIG_KERNEL_SMP


	p = (sc_signal_t*)_p;

	/* get target thread */
	this_p = process_find(p->pid);

	if(this_p == 0x0)
		return_errno(E_INVAL);

	this_t = list_find_safe(this_p->threads, tid, p->tid, &this_p->mtx);

	if(this_t == 0x0)
		return_errno(E_INVAL);

	/* trigger signal */
	DEBUG("thread %s.%d, signal %d, hdlr %p\n", this_p->name, this_t->tid, p->sig, this_t->parent->sig_hdlr);

	sched_lock();

#ifdef CONFIG_KERNEL_SMP
	core = sched_thread_core(this_t);

	if(this_t->state != RUNNING || core == PIR){
#endif // CONFIG_KERNEL_SMP

		// save current context signal_ctx_stack
		sig_ctx = kmalloc(sizeof(usignal_ctx_t));

		if(sig_ctx == 0x0)
			goto_errno(end, E_NOMEM);

		ctx = list_first(this_t->ctx_stack);

		sig_ctx->ctx_addr = ctx;
		memcpy(&sig_ctx->ctx, ctx, sizeof(thread_context_t));
		stack_push(this_t->signal_ctx_stack, sig_ctx);

		// inject signal handler
		thread_context_init(ctx, this_t, this_t->parent->sig_hdlr, 0x0, (void*)p->sig);
		ctx->next = sig_ctx->ctx.next;

#ifdef CONFIG_KERNEL_SMP
	}
	else{
		// signal core that is running the target thread
		(void)ipi_send(core, ipi_hdlr, p, sizeof(sc_signal_t));
	}
#endif // CONFIG_KERNEL_SMP


end:
	sched_unlock();

	return -errno;
}

static int sc_hdlr_signal_return(void *_p){
	thread_t *this_t;
	usignal_ctx_t *sig_ctx;
	thread_context_t *ctx;


	this_t = (thread_t*)sched_running();

	DEBUG("%s.%d\n", this_t->parent->name, this_t->tid);

	sched_lock();

	/* restore origianl context from signal_ctx_stack */
	sig_ctx = stack_pop(this_t->signal_ctx_stack);

	// use origianl context address since the current context, with all its stack
	// allocations needs to be dropped, i.e. all local variables used the signal hdlr
	ctx = sig_ctx->ctx_addr;

	memcpy(ctx, &sig_ctx->ctx, sizeof(thread_context_t));
	kfree(sig_ctx);

	(void)stack_pop(this_t->ctx_stack);
	stack_push(this_t->ctx_stack, ctx);

	sched_unlock();

	return E_OK;
}

#ifdef CONFIG_KERNEL_SMP
static void ipi_hdlr(void *data){
	(void)sc_hdlr_signal_send(data);
}
#endif // CONFIG_KERNEL_SMP
