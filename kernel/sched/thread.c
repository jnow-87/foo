/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/thread.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <sys/devtree.h>
#include <sys/mutex.h>
#include <sys/thread.h>
#include <sys/list.h>
#include <sys/stack.h>
#include <sys/errno.h>


/* global functions */
thread_t *thread_create(struct process_t *this_p, tid_t tid, thread_entry_t entry, void *thread_arg){
	thread_t *this_t;
	thread_ctx_t *ctx;
	devtree_memory_t const *stack;


	this_t = kcalloc(1, sizeof(thread_t));

	if(this_t == 0x0)
		goto err_0;

	/* set tid */
	if(tid == TID_MAX)
		goto_errno(err_1, E_LIMIT);

	this_t->tid = tid;

	mutex_init(&this_t->mtx, 0);

	/* set thread attributes */
	this_t->state = CREATED;
	this_t->affinity = CONFIG_CORE_MASK;
	this_t->priority = CONFIG_SCHED_PRIO_DEFAULT;
	this_t->entry = entry;
	this_t->parent = this_p;

	/* prepare stack */
	// NOTE the memory stack is ensured to exist by the build system
	stack = devtree_find_memory_by_name(&__dt_memory_root, "kernel-stack");

	this_t->stack = page_alloc(this_p, stack->size);

	if(this_t->stack == 0x0)
		goto_errno(err_1, E_NOMEM);

	/* init thread context */
	ctx = (thread_ctx_t*)(this_t->stack->phys_addr + stack->size - sizeof(thread_ctx_t));
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
	process_t *this_p;


	this_p = this_t->parent;

	list_rm_safe(this_p->threads, this_t, &this_p->mtx);

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


	this_t = (thread_t*)sched_running();
	ctx = stack_pop(this_t->ctx_stack);
	sig = list_first(this_t->signals);

	if(ctx->type == CTX_SIGRETURN)
		ctx = usignal_return(sig, this_t, ctx);

	sig = list_first(this_t->signals);

	if(sig && sig->pending && ctx->type == CTX_USER && this_t->parent->sig_hdlr)
		return usignal_entry(sig, this_t, ctx);

	return ctx;
}
