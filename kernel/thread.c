#include <config/config.h>
#include <arch/thread.h>
#include <kernel/thread.h>
#include <kernel/kmem.h>
#include <kernel/page.h>
#include <sys/list.h>
#include <sys/errno.h>


/* global functions */
thread_t *thread_create(struct process_t *this_p, thread_id_t tid, void *entry, void *thread_arg){
	thread_t *this_t;


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
	this_t->stack = page_alloc(this_p, CONFIG_THREAD_STACK_SIZE);

	if(this_t->stack == 0)
		goto_errno(err_1, E_NOMEM);

	list_add_tail(this_p->memory.pages, this_t->stack);

	/* init thread context */
	this_t->ctx = thread_context_init(this_t, thread_arg);

	if(this_t->ctx == 0)
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
	page_free(this_t->parent, this_t->stack);
	kfree(this_t);
}
