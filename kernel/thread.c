#include <config/config.h>
#include <arch/thread.h>
#include <kernel/thread.h>
#include <kernel/kmem.h>
#include <kernel/page.h>
#include <kernel/lock.h>
#include <sys/list.h>
#include <sys/errno.h>


/* global functions */
thread_t *thread_create(struct process_t *this_p, thread_id_t tid, void *entry, void *thread_arg){
	thread_t *this_t;
	void *proc_entry;


	this_t = kmalloc(sizeof(thread_t));

	if(this_t == 0)
		goto_errno(err_0, E_NOMEM);

	/* set tid */
	if(tid == THREAD_ID_MAX)
		goto_errno(err_1, E_LIMIT);

	this_t->tid = tid;

	/* set thread attributes */
	this_t->parent = this_p;
	this_t->entry = entry;
	this_t->state = CREATED;
	this_t->affinity = CONFIG_CORE_MASK;
	this_t->priority = 0;

	/* prepare stack */
	this_t->stack = page_alloc(this_p, CONFIG_KERNEL_STACK_SIZE);

	if(this_t->stack == 0)
		goto_errno(err_1, E_NOMEM);

	klock();
	list_add_tail(this_p->memory.pages, this_t->stack);
	kunlock();

	/* init thread context */
	proc_entry = entry;

	if(tid != 0)
		proc_entry = list_first(this_p->threads)->entry;

	this_t->ctx_lst = 0x0;
	thread_context_enqueue(this_t, thread_context_init(this_t, proc_entry, thread_arg));

	if(this_t->ctx_lst == 0)
		goto_errno(err_2, E_INVAL);

	return this_t;


err_2:
	kfree(this_t->stack);

err_1:
	kfree(this_t);

err_0:
	return 0;
}

void thread_destroy(struct thread_t *this_t){
	process_t *this_p;


	this_p = this_t->parent;

	list_rm(this_p->memory.pages, this_t->stack);
	page_free(this_p, this_t->stack);
	kfree(this_t);
}

void thread_context_enqueue(thread_t *this_t, thread_context_t *ctx){
	klock();
	list_add_tail(this_t->ctx_lst, ctx);
	kunlock();
}

thread_context_t *thread_context_dequeue(thread_t *this_t){
	thread_context_t *ctx;


	klock();
	ctx = list_last(this_t->ctx_lst);
	list_rm(this_t->ctx_lst, ctx);
	kunlock();

	return ctx;
}
