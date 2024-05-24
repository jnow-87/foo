/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <sys/devtree.h>
#include <sys/devicetree.h>
#include <sys/mutex.h>
#include <sys/thread.h>
#include <sys/list.h>
#include <sys/stack.h>
#include <sys/errno.h>


/* global functions */
thread_t *thread_create(struct process_t *this_p, tid_t tid, thread_entry_t entry, void *thread_arg){
	thread_t *this_t;
	thread_ctx_t *ctx;


	this_t = kcalloc(1, sizeof(thread_t));

	if(this_t == 0x0)
		goto err_0;

	/* set tid */
	if(tid == TID_MAX)
		goto_errno(err_1, E_LIMIT);

	this_t->tid = tid;

	mutex_init(&this_t->mtx, MTX_NOINT);

	/* set thread attributes */
	this_t->state = CREATED;
	this_t->affinity = DEVTREE_ARCH_CORE_MASK;
	this_t->priority = CONFIG_SCHED_PRIO_DEFAULT;
	this_t->entry = entry;
	this_t->parent = this_p;

	/* prepare stack */
	// NOTE the memory stack is ensured to exist by the build system
	this_t->stack = page_alloc(this_p, CONFIG_STACK_SIZE);

	if(this_t->stack == 0x0)
		goto_errno(err_1, E_NOMEM);

	/* init thread context */
	ctx = (thread_ctx_t*)(this_t->stack->phys_addr + CONFIG_STACK_SIZE - sizeof(thread_ctx_t));
	thread_ctx_init(ctx, this_t, entry, thread_arg);

	stack_push(this_t->ctx_stack, ctx);

	if(this_t->ctx_stack == 0x0)
		goto_errno(err_2, E_INVAL);

	list_add_tail_safe(this_p->threads, this_t, &this_p->mtx);

	return this_t;


err_2:
	page_free(this_p, this_t->stack);

err_1:
	kfree(this_t);

err_0:
	return 0;
}

void thread_destroy(struct thread_t *this_t){
	process_t *this_p = this_t->parent;
	thread_dtor_t *dtor;


	list_rm_safe(this_p->threads, this_t, &this_p->mtx);

	list_for_each(this_t->dtors, dtor){
		dtor->hdlr(this_t, dtor->payload);
		kfree(dtor);
	}

	page_free(this_p, this_t->stack);
	usignal_destroy(this_t);

	kfree(this_t);
}

void thread_ctx_push(thread_ctx_t *ctx){
	stack_push(sched_running()->ctx_stack, ctx);
}

thread_ctx_t *thread_ctx_pop(void){
	thread_t *this_t;
	thread_ctx_t *ctx;
	usignal_t *sig;


	this_t = sched_running();
	ctx = stack_pop(this_t->ctx_stack);
	sig = list_first_safe(this_t->signals, &this_t->mtx);

	if(ctx->type == CTX_SIGRETURN)
		ctx = usignal_return(sig, this_t, ctx);

	sig = list_first_safe(this_t->signals, &this_t->mtx);

	if(sig && sig->pending && ctx->type == CTX_USER && this_t->parent->sig_hdlr)
		return usignal_entry(sig, this_t, ctx);

	return ctx;
}

int thread_dtor_register(thread_t *this_t, thread_dtor_hdlr_t hdlr, void *payload){
	thread_dtor_t *dtor;


	dtor = kmalloc(sizeof(thread_dtor_t));

	if(dtor == 0x0)
		return -errno;

	dtor->hdlr = hdlr;
	dtor->payload = payload;

	list_add_tail_safe(this_t->dtors, dtor, &this_t->mtx);

	return 0;
}

void thread_dtor_release(thread_t *this_t, thread_dtor_hdlr_t hdlr, void *payload){
	thread_dtor_t *dtor;


	mutex_lock(&this_t->mtx);

	list_for_each(this_t->dtors, dtor){
		if(dtor->hdlr != hdlr || dtor->payload != payload)
			continue;

		list_rm(this_t->dtors, dtor);
		kfree(dtor);
	}

	mutex_unlock(&this_t->mtx);
}
