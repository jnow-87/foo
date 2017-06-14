#include <config/config.h>
#include <kernel/process.h>
#include <kernel/thread.h>
#include <kernel/sched.h>
#include <kernel/kmem.h>
#include <kernel/binloader.h>
#include <sys/errno.h>
#include <sys/list.h>
#include <sys/string.h>


/* macros */
#define MAX_MEMORY_ENTRIES	32


/* global functions */
process_t *process_create(void *binary, bin_type_t bin_type, char const *name, char const *args, fs_node_t *cwd){
	void *entry;
	char *_args;
	process_t *this_p;
	thread_t *this_t;
	unsigned int i;


	/* allocate process structure */
	this_p = kmalloc(sizeof(process_t));

	if(this_p == 0)
		goto_errno(err_0, E_NOMEM);

	/* get pid */
	this_p->pid = 0;

	if(!list_empty(process_table))
		this_p->pid = list_last(process_table)->pid + 1;

	if(this_p->pid == PROCESS_ID_MAX)
		goto_errno(err_1, E_LIMIT);

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

#ifdef CONFIG_KERNEL_SMP
	mutex_init(&this_p->memory.mtx);
#endif // CONFIG_KERNEL_SMP

	/* init arguments */
	// allocate args
	this_p->args = kmalloc(strlen(args) + 1);

	if(this_p->args == 0)
		goto_errno(err_2, E_NOMEM);

	strcpy(this_p->args, args);

	// identify number of arguments
	this_p->argc = (*args != 0) ? 1 : 0;

	while(*args != 0){
		// skip blanks
		while(*args != 0 && (*args == ' ' || *args == '\t'))
			++args;

		if(*args != 0)
			++this_p->argc;

		// skip non-blanks
		while(*args != 0 && (*args != ' ' && *args != '\t'))
			++args;
	}

	// allocate argv
	this_p->argv = 0;

	if(this_p->argc > 0){
		this_p->argv = kmalloc(sizeof(char*) * this_p->argc);

		if(this_p->argv == 0)
			goto_errno(err_3, E_NOMEM);

		memset(this_p->argv, 0x0, sizeof(char*) * this_p->argc);

		_args = this_p->args;
		i = 0;

		while(*_args != 0){
			// skip blanks
			while(*_args != 0 && (*_args == ' ' || *_args == '\t'))
				++_args;

			if(*_args != 0){
				this_p->argv[i] = _args;
				++i;
			}

			// skip non-blanks
			while(*_args != 0 && (*_args != ' ' && *_args != '\t'))
				++_args;

			// terminate current argument
			if(*_args == 0)
				break;

			*(_args++) = 0;
		}
	}

	/* init file system handles */
	this_p->fds = 0x0;
	this_p->cwd = 0x0;	// TODO implement -- call fs_get_cwd()

	/* load binary */
	entry = 0x0;

	if(bin_load(binary, bin_type, this_p, &entry) != E_OK)
		goto err_4;

	if(entry == 0x0)
		goto err_4;

	/* create first thread */
	this_t = thread_create(this_p, 0, entry);

	if(this_t == 0)
		goto err_4;

	this_p->threads = 0x0;
	list_add_tail(this_p->threads, this_t);

	return this_p;


err_4:
	kfree(this_p->argv);

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


	/* destroy all threads */
	list_for_each(this_p->threads, this_t){
		list_rm(this_p->threads, this_t);
		thread_destroy(this_t);
	}

	/* clear file system handles */
	// TODO implement -- call fs_cleanup_fds()

	/* free arguments */
	kfree(this_p->argv);
	kfree(this_p->args);

	/* free process memory */
	list_for_each(this_p->memory.pages, page){
		list_rm(this_p->memory.pages, page);
		page_free(this_p, page);
	}

	/* free process */
	kfree(this_p->name);
	kfree(this_p);
}
