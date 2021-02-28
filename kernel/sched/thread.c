/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/thread.h>
#include <kernel/thread.h>
#include <kernel/memory.h>
#include <sys/devtree.h>
#include <sys/thread.h>
#include <sys/list.h>
#include <sys/stack.h>
#include <sys/errno.h>


/* global functions */
thread_t *thread_create(struct process_t *this_p, tid_t tid, thread_entry_t entry, void *thread_arg){
	thread_t *this_t;
	thread_ctx_t *ctx;
	devtree_memory_t const *stack;


	this_t = kmalloc(sizeof(thread_t));

	if(this_t == 0x0)
		goto err_0;

	/* set tid */
	if(tid == TID_MAX)
		goto_errno(err_1, E_LIMIT);

	this_t->tid = tid;

	/* set thread attributes */
	this_t->parent = this_p;
	this_t->entry = entry;
	this_t->state = CREATED;
	this_t->affinity = CONFIG_CORE_MASK;
	this_t->priority = 0;

	/* prepare stack */
	// NOTE the memory stack is ensured to exist by the build system
	stack = devtree_find_memory_by_name(&__dt_memory_root, "kernel_stack");

	this_t->stack = page_alloc(this_p, stack->size);

	if(this_t->stack == 0x0)
		goto_errno(err_1, E_NOMEM);

	/* init thread context */
	this_t->signal_ctx_stack = 0x0;
	this_t->ctx_stack = 0x0;

	ctx = (thread_ctx_t*)(this_t->stack->phys_addr + stack->size - sizeof(thread_ctx_t));
	thread_context_init(ctx, this_t, entry, thread_arg);

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
	usignal_ctx_t *sig_ctx;


	this_p = this_t->parent;

	list_rm_safe(this_p->threads, this_t, &this_p->mtx);

	page_free(this_p, this_t->stack);

	list_for_each(this_t->signal_ctx_stack, sig_ctx)
		kfree(sig_ctx);

	kfree(this_t);
}
