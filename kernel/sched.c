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
#include <kernel/syscall.h>
#include <kernel/panic.h>
#include <sys/errno.h>
#include <sys/mutex.h>
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

static int sc_hdlr_exit(void *p);

static int sc_hdlr_thread_create(void *p);
static int sc_hdlr_thread_info(void *p);
static int sc_hdlr_nice(void *p);

static int sc_hdlr_process_create(void *p);
static int sc_hdlr_process_info(void *p);

static int sc_hdlr_sched_yield(void *p);

static void transition(thread_t *this_t, thread_state_t queue);


/* global variables */
process_t *process_table = 0;


/* static variables */
static sched_queue_t *sched_queues[NTHREADSTATES] = { 0x0 };
static thread_t *running[CONFIG_NCORES] = { 0x0 };
static mutex_t sched_mtx = MUTEX_INITIALISER();


/* global functions */
void sched_tick(void){
	int_type_t imask;
	thread_t *this_t;


	imask = int_enable(INT_NONE);
	mutex_lock(&sched_mtx);

	// NOTE The running thread might already transitioned to a different
	//		state, e.g. through the kernel signal mechanism
	if(running[PIR]->state == RUNNING)
		transition(running[PIR], READY);

	this_t = list_first(sched_queues[READY])->thread;

	if(this_t == 0x0)
		kpanic(0x0, "no ready thread\n");

	transition(this_t, RUNNING);
	running[PIR] = this_t;

	mutex_unlock(&sched_mtx);
	int_enable(imask);

	// TODO if the thread is not actually switched and the only ready thread
	// 		is a kernel thread suspend the core
}

thread_t const *sched_running(void){
	if(running[PIR] == 0x0)
		kpanic(0x0, "no running thread\n");

	return running[PIR];
}

void sched_yield(void){
	char dummy;


	if(int_enabled() == INT_NONE)
		kpanic(sched_running(), "interrupts are disabled, syscall not allowed\n");

	// actual thread switches are only performed in interrupt
	// service routines as it is required for syscalls
	sc(SC_SCHEDYIELD, &dummy);
}

void sched_pause(void){
	int_type_t imask;


	imask = int_enable(INT_NONE);
	mutex_lock(&sched_mtx);
	transition((thread_t*)sched_running(), WAITING);
	mutex_unlock(&sched_mtx);
	int_enable(imask);
}

void sched_wake(thread_t *this_t){
	int_type_t imask;


	imask = int_enable(INT_NONE);
	mutex_lock(&sched_mtx);
	transition(this_t, READY);
	mutex_unlock(&sched_mtx);
	int_enable(imask);
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
	r |= sc_register(SC_NICE, sc_hdlr_nice);
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
	}

	// add kernel threads to running queue
	list_for_each(this_p->threads, this_t){
		transition(this_t, READY);
		transition(this_t, RUNNING);

		running[PIR] = this_t;
	}

	/* create init process */
	// create init processs
	this_p = process_create(kopt.init_bin, kopt.init_type, "init", kopt.init_arg, fs_root);

	if(this_p == 0)
		goto err;

	list_add_tail(process_table, this_p);

	// add first thread to ready queue
	transition(this_p->threads, READY);

	return E_OK;


err:
	/* NOTE cleanup in case of an error is not required, since the
	 * 		kernel will stop anyways if any of the init calls fails
	 */
	return_errno(errno);
}

kernel_init(2, init);

static int sc_hdlr_exit(void *p){
	process_t *this_p;
	thread_t const *this_t;
	sched_queue_t *e;


	this_t = sched_running();
	this_p = this_t->parent;

	DEBUG("thread %s:%u exit with status %d\n", this_p->name, this_t->tid, *((int*)p));

	sched_tick();

	mutex_lock(&sched_mtx);

	e = list_find(sched_queues[this_t->state], thread, this_t);

	if(e == 0x0)
		kpanic(this_t, "thread not found in supposed sched queue %d\n", this_t->state);

	list_rm(sched_queues[this_t->state], e);
	kfree(e);

	mutex_unlock(&sched_mtx);

	mutex_lock(&this_p->mtx);
	list_rm(this_p->threads, this_t);
	mutex_unlock(&this_p->mtx);

	thread_destroy((thread_t*)this_t);

	mutex_lock(&sched_mtx);

	if(list_empty(this_p->threads)){
		list_rm(process_table, this_p);
		process_destroy(this_p);
	}

	mutex_unlock(&sched_mtx);

	return E_OK;
}

static int sc_hdlr_thread_create(void *_p){
	sc_thread_t *p;
	thread_t *new;
	process_t *this_p;


	p = (sc_thread_t*)_p;
	this_p = sched_running()->parent;

	DEBUG("create thread for \"%s\" at %p, arg %p\n", this_p->name, p->entry, p->arg);

	new = thread_create(this_p, list_last(this_p->threads)->tid + 1, p->entry, p->arg);

	if(new == 0x0)
		goto end;

	mutex_lock(&sched_mtx);

	list_add_tail(this_p->threads, new);
	transition(new, READY);

	mutex_unlock(&sched_mtx);


end:
	p->errno = errno;
	p->tid = (new == 0x0) ? 0 : new->tid;

	return E_OK;
}

static int sc_hdlr_thread_info(void *_p){
	sc_thread_t *p;
	thread_t const *this_t;


	p = (sc_thread_t*)_p;
	this_t = sched_running();

	p->tid = this_t->tid;
	p->priority = this_t->priority;
	p->affinity = this_t->affinity;
	p->errno = E_OK;

	return E_OK;
}

static int sc_hdlr_nice(void *_p){
	sc_thread_t *p;
	thread_t const *this_t;


	p = (sc_thread_t*)_p;
	this_t = sched_running();

	((thread_t*)this_t)->priority += p->priority;

	p->priority = this_t->priority;
	p->errno = E_OK;

	return E_OK;
}

static int sc_hdlr_process_create(void *_p){
	sc_process_t *p = (sc_process_t*)_p;
	char name[p->name_len + 1];
	char args[p->args_len + 1];
	process_t *this_p,
			  *new;


	/* process arguments */
	this_p = sched_running()->parent;

	if(copy_from_user(name, p->name, p->name_len + 1, this_p) != E_OK)
		goto end;

	if(copy_from_user(args, p->args, p->args_len + 1, this_p) != E_OK)
		goto end;

	/* create process */
	DEBUG("create process \"%s\" with args \"%s\"\n", name, args);

	new = process_create(p->binary, p->bin_type, p->name, p->args, this_p->cwd);

	if(new == 0x0)
		goto end;

	mutex_lock(&sched_mtx);

	list_add_tail(process_table, new);
	transition(list_first(new->threads), READY);

	mutex_unlock(&sched_mtx);


end:
	p->errno = errno;
	p->pid = (new == 0x0) ? 0 : new->pid;

	return E_OK;
}

static int sc_hdlr_process_info(void *_p){
	sc_process_t *p;
	process_t *this_p;


	p = (sc_process_t*)_p;
	this_p = sched_running()->parent;

	p->pid = this_p->pid;
	p->errno = E_OK;

	return E_OK;
}

static int sc_hdlr_sched_yield(void *p){
	sched_tick();
	return E_OK;
}

/**
 * \brief
 *
 *                         kill
 *            WAITING ----------------\
 *             |   A                  |
 *             |   | sig wait         |
 *    sig_wake |   |                  |
 *             |   |           exit   |
 *             |  RUNNING ------------|
 *             |  |A                  |
 *             |  || sched            |
 *             |  ||                  |
 *             V  V|       kill       V
 * CREATED --> READY --------------> DEAD
 *
 * TODO kill is not yet implemented
 *
 * \pre	calls to transition are protected through sched_mtx
 */
static void transition(thread_t *this_t, thread_state_t queue){
	thread_state_t s;
	sched_queue_t *e;


	/* check for invalid state transition */
	s = this_t->state;

	if((queue == CREATED)
	|| (queue == READY && s == DEAD)
	|| (queue == RUNNING && s != READY)
	|| (queue == WAITING && s != RUNNING)
	|| (queue == DEAD && s == CREATED)
	){
		kpanic(this_t, "invalid scheduler transition %u -> %u\n", s, queue);
	}

	/* perform transition */
	if(this_t->state == CREATED){
		e = kmalloc(sizeof(sched_queue_t));

		if(e == 0x0)
			kpanic(this_t, "out of memory\n");

		e->thread = this_t;
	}
	else{
		e = list_find(sched_queues[this_t->state], thread, this_t);

		if(e == 0x0)
			kpanic(this_t, "thread not found in supposed sched queue %d\n", this_t->state);

		list_rm(sched_queues[this_t->state], e);
	}

	list_add_tail(sched_queues[queue], e);
	this_t->state = queue;
}
