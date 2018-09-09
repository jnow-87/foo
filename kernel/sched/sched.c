#include <config/config.h>
#include <arch/interrupt.h>
#include <arch/syscall.h>
#include <arch/core.h>
#include <arch/memory.h>
#include <kernel/opt.h>
#include <kernel/init.h>
#include <kernel/sched.h>
#include <kernel/memory.h>
#include <kernel/rootfs.h>
#include <kernel/syscall.h>
#include <kernel/panic.h>
#include <kernel/task.h>
#include <kernel/csection.h>
#include <sys/errno.h>
#include <sys/list.h>
#include "sched.h"


/* types */
typedef struct sched_queue_t{
	struct sched_queue_t *prev,
						 *next;

	thread_t *thread;
} sched_queue_t;


/* local/static prototypes */
static void task_cleanup(void *p);


/* global variables */
sched_queue_t *sched_queues[NTHREADSTATES] = { 0x0 };
csection_lock_t sched_lock = CSECTION_INITIALISER();


/* static variables */
static thread_t *running[CONFIG_NCORES] = { 0x0 };


/* global functions */
void sched_tick(void){
	thread_t *this_t;


	csection_lock(&sched_lock);

	// NOTE The running thread might have already transitioned to a
	// 		different state, e.g. through the kernel signal mechanism
	// 		or a kill
	if(running[PIR]->state == RUNNING)
		sched_transition(running[PIR], READY);

	this_t = list_first(sched_queues[READY])->thread;

	if(this_t == 0x0)
		kpanic(0x0, "no ready thread\n");

	sched_transition(this_t, RUNNING);
	running[PIR] = this_t;

	csection_unlock(&sched_lock);

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
	(void)sc(SC_SCHEDYIELD, &dummy);
}

void sched_pause(void){
	csection_lock(&sched_lock);
	sched_transition((thread_t*)sched_running(), WAITING);
	csection_unlock(&sched_lock);
}

void sched_wake(thread_t *this_t){
	csection_lock(&sched_lock);
	sched_transition(this_t, READY);
	csection_unlock(&sched_lock);
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
 * \pre	calls to sched_transition are protected through sched_lock
 */
void sched_transition(thread_t *this_t, thread_state_t queue){
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


/* local functions */
static int init(void){
	unsigned int i;
	process_t *this_p;
	thread_t *this_t;


	/* create kernel process */
	this_p = kmalloc(sizeof(process_t));

	if(this_p == 0x0)
		goto_errno(err, E_NOMEM);

	memset(this_p, 0x0, sizeof(process_t));
	this_p->name = (char*)("kernel");

	/* create kernel thread */
	// allocate one thread per core
	for(i=0; i<CONFIG_NCORES; i++){
		this_t = kmalloc(sizeof(thread_t));

		if(this_t == 0x0)
			goto_errno(err, E_NOMEM);

		this_t->tid = i;
		this_t->state = CREATED;
		this_t->priority = CONFIG_SCHED_PRIO_DEFAULT;
		this_t->affinity = (0x1 << i);
		this_t->parent = this_p;

		this_t->entry = 0x0;	// kernel threads are already running
		this_t->ctx = 0x0;		// kernel thread context is set automatically once
								// the thread is interrupted for the first time
		this_t->stack = 0x0;	// stack pages are only relevant for user space,
								// since the kernel has a separate memory management

		list_add_tail(this_p->threads, this_t);
	}

	// add kernel threads to running queue
	list_for_each(this_p->threads, this_t){
		sched_transition(this_t, READY);
		sched_transition(this_t, RUNNING);

		running[PIR] = this_t;
	}

	/* create init process */
	// create init processs
	this_p = process_create(kopt.init_bin, kopt.init_type, "init", kopt.init_arg, fs_root);

	if(this_p == 0x0)
		goto err;

	// add first thread to ready queue
	sched_transition(this_p->threads, READY);

	/* create cleanup task */
	if(ktask_create(task_cleanup, 0x0, 0, 0x0, true) != 0)
		goto err;

	return E_OK;


err:
	/* NOTE cleanup in case of an error is not required, since the
	 * 		kernel will stop anyways if any of the init calls fails
	 */
	return -errno;
}

kernel_init(2, init);

/**
 * \brief	recurring task used to cleanup terminated threads
 * 			and processes
 */
static void task_cleanup(void *p){
	sched_queue_t *e;
	process_t *this_p;
	thread_t *this_t;


	/* NOTE	use while rather than list_for_each to keep the critical
	 * 		section as short as possible, reducing the time of disabled
	 * 		interrupts
	 */
	while(1){
		/* remove thread scheduler queue entry */
		csection_lock(&sched_lock);

		e = list_first(sched_queues[DEAD]);

		if(e == 0x0)
			break;

		this_t = e->thread;
		this_p = this_t->parent;

		list_rm(sched_queues[this_t->state], e);
		kfree(e);

		csection_unlock(&sched_lock);

		/* destroy thread and potentially parent process */
		DEBUG("destroy thread %s.%d\n", this_p->name, this_t->tid);

		thread_destroy(this_t);

		if(list_empty_safe(this_p->threads, &this_p->mtx))
			process_destroy(this_p);
	}

	csection_unlock(&sched_lock);
}
