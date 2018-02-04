#include <config/config.h>
#include <arch/core.h>
#include <kernel/process.h>
#include <kernel/thread.h>
#include <kernel/sched.h>
#include <kernel/kmem.h>
#include <kernel/rootfs.h>
#include <kernel/binloader.h>
#include <sys/errno.h>
#include <sys/list.h>
#include <sys/string.h>
#include <sys/mutex.h>


/* static variables */
static process_t *process_table = 0;
static mutex_t ptable_mtx = MUTEX_INITIALISER();


/* global functions */
process_t *process_create(void *binary, bin_type_t bin_type, char const *name, char const *args, fs_node_t *cwd){
	void *entry;
	process_t *this_p;
	thread_t *this_t;


	/* allocate process structure */
	this_p = kmalloc(sizeof(process_t));

	if(this_p == 0)
		goto_errno(err_0, E_NOMEM);

	/* get pid */
	this_p->pid = 0;

	mutex_lock(&ptable_mtx);

	if(!list_empty(process_table))
		this_p->pid = list_last(process_table)->pid + 1;

	mutex_unlock(&ptable_mtx);

	if(this_p->pid == PID_MAX)
		goto_errno(err_1, E_LIMIT);

	/* init mutex */
	mutex_init(&this_p->mtx, MTX_NESTED);

	/* set process attributes */
	this_p->priority = CONFIG_SCHED_PRIO_DEFAULT;
	this_p->affinity = CONFIG_SCHED_AFFINITY_DEFAULT;

	this_p->name = kmalloc(strlen(name) + 1);

	if(this_p->name == 0)
		goto_errno(err_1, E_NOMEM);

	strcpy(this_p->name, name);

	/* init memory */
	this_p->memory.pages = 0x0;

#ifdef CONFIG_KERNEL_VIRT_MEM
	/* TODO
	 * 	implementation for process virtual memory required so far the memblock_t functions cannot be used since
	 * 	they require an actual physical backing of the managed memory
	 */
#endif // CONFIG_KERNEL_VIRT_MEM

	/* init argument string */
	this_p->args = kmalloc(strlen(name) + 1 + strlen(args) + 1);

	if(this_p->args == 0)
		goto_errno(err_2, E_NOMEM);

	strcpy(this_p->args, name);
	strcpy(this_p->args + strlen(name), " ");
	strcpy(this_p->args + strlen(name) + 1, args);

	/* init file system handles */
	this_p->fds = 0x0;
	this_p->cwd = cwd;

	/* load binary */
	entry = 0x0;

	if(bin_load(binary, bin_type, this_p, &entry) != E_OK)
		goto err_3;

	/* create first thread */
	this_p->threads = 0x0;
	this_t = thread_create(this_p, 0, entry, (void*)this_p->args);

	if(this_t == 0)
		goto err_3;

	/* update process table */
	list_add_tail_safe(process_table, this_p, &ptable_mtx);

	return this_p;


err_3:
	kfree(this_p->args);

err_2:
	kfree(this_p->name);

err_1:
	kfree(this_p);

err_0:
	return 0;
}

void process_destroy(process_t *this_p){
	thread_t *this_t;
	page_t *page;
	fs_filed_t *fd;
	fs_ops_t *ops;


	list_rm_safe(process_table, this_p, &ptable_mtx);

	/* destroy all threads */
	list_for_each(this_p->threads, this_t)
		thread_destroy(this_t);

	/* clear file system handles */
	list_for_each(this_p->fds, fd){
		ops = fd->node->ops;

		fs_lock();

		if(ops->close != 0x0)	ops->close(fd, this_p);
		else					fs_fd_free(fd, this_p);

		fs_unlock();
	}

	/* free arguments */
	kfree(this_p->args);

	/* free process memory */
	list_for_each(this_p->memory.pages, page)
		page_free(this_p, page);

	/* free process */
	kfree(this_p->name);
	kfree(this_p);
}
