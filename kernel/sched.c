#include <config/config.h>
#include <arch/interrupt.h>
#include <arch/syscall.h>
#include <arch/mem.h>
#include <kernel/init.h>
#include <kernel/opt.h>
#include <kernel/sched.h>
#include <kernel/process.h>
#include <kernel/thread.h>
#include <kernel/kmem.h>
#include <kernel/rootfs.h>
#include <kernel/lock.h>
#include <kernel/syscall.h>
#include <kernel/panic.h>
#include <sys/errno.h>
#include <sys/list.h>
#include <sys/string.h>


/* types */
typedef struct sched_queue_t{
	thread_t *thread;

	struct sched_queue_t *prev,
						 *next;
} sched_queue_t;


/* local/static prototypes */
static int init(void);

static int sc_hdlr_exit(void *p, thread_t const *this_t);
static int sc_hdlr_thread_create(void *p, thread_t const *this_t);
static int sc_hdlr_thread_info(void *p, thread_t const *this_t);
static int sc_hdlr_process_create(void *p, thread_t const *this_t);
static int sc_hdlr_process_info(void *p, thread_t const *this_t);
static int sc_hdlr_sched_yield(void *p, thread_t const *this_t);

static int sched_queue_add(sched_queue_t **queue, thread_t *this_t);
static int sched_queue_rm(sched_queue_t **queue, thread_t const *this_t);


/* global variables */
process_t *process_table = 0;


/* static variables */
static thread_t *current_thread[CONFIG_NCORES] = { 0x0 };
static sched_queue_t *queue_ready = 0,
					 *queue_waiting = 0;


/* global functions */
thread_t const *sched_running(void){
	return current_thread[PIR];
}

void sched_tick(void){
	static sched_queue_t *e = 0;


	/* temporary simple thread select */
	if(e == 0)
		e = list_first(queue_ready);

	current_thread[PIR] = e->thread;
	e = e->next;

	// TODO check for next thread
	// TODO switch thread or goto sleep
}

void sched_yield(void){
	int p;


	// actual thread switches are only performed in interrupt
	// service routines as it is required for syscalls
	sc(SC_SCHEDYIELD, &p);
}


/* local functions */
static int init(void){
	unsigned int i;
	int r;
	process_t *this_p;
	thread_t *this_t;


	/* register syscalls */
	r = 0;

	r |= sc_register(SC_EXIT, sc_hdlr_exit);
	r |= sc_register(SC_THREADCREATE, sc_hdlr_thread_create);
	r |= sc_register(SC_THREADINFO, sc_hdlr_thread_info);
	r |= sc_register(SC_PROCCREATE, sc_hdlr_process_create);
	r |= sc_register(SC_PROCINFO, sc_hdlr_process_info);
	r |= sc_register(SC_SCHEDYIELD, sc_hdlr_sched_yield);

	if(r != 0)
		goto err;

	/* create kernel process */
	this_p = kmalloc(sizeof(process_t));

	if(this_p == 0)
		goto_errno(err, E_NOMEM);

	memset(this_p, 0x0, sizeof(process_t));

	this_p->name = (char*)("kernel");
	this_p->affinity = CONFIG_SCHED_AFFINITY_DEFAULT;
	this_p->priority = CONFIG_SCHED_PRIO_DEFAULT;
	this_p->cwd = fs_root;

	list_add_tail(process_table, this_p);

	/* create kernel thread */
	// allocate one thread per core
	for(i=0; i<CONFIG_NCORES; i++){
		this_t = kmalloc(sizeof(thread_t));

		if(this_t == 0)
			goto_errno(err, E_NOMEM);

		this_t->tid = i;
		this_t->state = CREATED;
		this_t->priority = CONFIG_SCHED_PRIO_DEFAULT;
		this_t->affinity = (0x1 << i);
		this_t->parent = this_p;

		this_t->entry = 0x0;	// kernel threads are already running
		this_t->ctx_lst = 0x0;	// kernel thread context is set automatically once
								// the thread is interrupted for the first time
		this_t->stack = 0x0;	// stack pages are only relevant for user space,
								// since the kernel has a separate memory management

		list_add_tail(this_p->threads, this_t);
		current_thread[i] = this_t;
	}

	// add kernel threads to ready queue
	list_for_each(this_p->threads, this_t){
		if(sched_queue_add(&queue_ready, this_t) != E_OK)
			goto err;
	}

	/* create init process */
	// create init processs
	this_p = process_create(kopt.init_bin, kopt.init_type, "init", kopt.init_arg, fs_root);

	if(this_p == 0)
		goto err;

	list_add_tail(process_table, this_p);

	// add first thread to ready queue
	if(sched_queue_add(&queue_ready, this_p->threads) != E_OK)
		goto err;

	return E_OK;


err:
	/* XXX: cleanup in case of an error is not required, since the kernel will stop
	 * anyways if any of the init calls fails
	 */
	return_errno(errno);
}

kernel_init(2, init);

static int sc_hdlr_exit(void *p, thread_t const *this_t){
	process_t *this_p;


	this_p = this_t->parent;

	DEBUG("thread %s:%u exit with status %d\n", this_p->name, this_t->tid, *((int*)p));

	klock();

	if(sched_queue_rm(&queue_ready, this_t) != E_OK)
		kpanic(this_t, "unable to remove thread from sched queue (%d)\n", errno);

	list_rm(this_p->threads, this_t);

	kunlock();

	thread_destroy((thread_t*)this_t);

	if(list_empty(this_p->threads)){
		list_rm(process_table, this_p);
		process_destroy(this_p);
	}

	sched_tick();

	return E_OK;
}

static int sc_hdlr_thread_create(void *_p, thread_t const *this_t){
	sc_thread_t *p;
	thread_t *new;
	process_t *this_p;


	p = (sc_thread_t*)_p;
	this_p = this_t->parent;

	DEBUG("create thread for \"%s\" at %p, arg %p\n", this_p->name, p->entry, p->arg);

	new = thread_create(this_p, list_last(this_p->threads)->tid + 1, p->entry, p->arg);

	if(new == 0x0)
		goto end;

	klock();

	list_add_tail(this_p->threads, new);

	if(sched_queue_add(&queue_ready, new) != E_OK)
		goto err;

	kunlock();

	goto end;

err:
	kunlock();
	thread_destroy(new);
	new = 0x0;

end:
	p->errno = errno;
	p->tid = (new == 0x0) ? 0 : new->tid;

	return E_OK;
}

static int sc_hdlr_thread_info(void *_p, thread_t const *this_t){
	sc_thread_t *p;


	p = (sc_thread_t*)_p;

	p->tid = this_t->tid;
	p->priority = this_t->priority;
	p->affinity = this_t->affinity;
	p->errno = E_OK;

	return E_OK;
}

static int sc_hdlr_process_create(void *_p, thread_t const *this_t){
	sc_process_t *p = (sc_process_t*)_p;
	char name[p->name_len + 1];
	char args[p->args_len + 1];
	process_t *new;


	/* process arguments */
	if(copy_from_user(name, p->name, p->name_len + 1, this_t->parent) != E_OK)
		goto end;

	if(copy_from_user(args, p->args, p->args_len + 1, this_t->parent) != E_OK)
		goto end;

	/* create process */
	DEBUG("create process \"%s\" with args \"%s\"\n", name, args);

	new = process_create(p->binary, p->bin_type, p->name, p->args, this_t->parent->cwd);

	if(new == 0x0)
		goto end;

	klock();

	list_add_tail(process_table, new);

	if(sched_queue_add(&queue_ready, new->threads) != E_OK)
		goto err;

	kunlock();

	goto end;

err:
	kunlock();
	process_destroy(new);
	new = 0x0;

end:
	p->errno = errno;
	p->pid = (new == 0x0) ? 0 : new->pid;

	return E_OK;
}

static int sc_hdlr_process_info(void *_p, thread_t const *this_t){
	sc_process_t *p;


	p = (sc_process_t*)_p;

	p->pid = this_t->parent->pid;
	p->errno = E_OK;

	return E_OK;
}

static int sc_hdlr_sched_yield(void *_p, thread_t const *this_t){
	sched_tick();
	return E_OK;
}

static int sched_queue_add(sched_queue_t **queue, thread_t *this_t){
	sched_queue_t *e;


	e = kmalloc(sizeof(sched_queue_t));

	if(e == 0)
		goto err;

	klock();

	e->thread = this_t;
	list_add_tail(*queue, e);

	kunlock();

	return E_OK;


err:
	return_errno(E_NOMEM);
}

static int sched_queue_rm(sched_queue_t **queue, thread_t const *this_t){
	sched_queue_t *e;


	klock();

	e = list_find(*queue, thread, this_t);

	if(e == 0x0)
		goto_errno(err, E_INVAL);

	list_rm(*queue, e);
	kfree(e);

	kunlock();

	return E_OK;


err:
	kunlock();

	return errno;
}
