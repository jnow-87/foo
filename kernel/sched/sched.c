/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <kernel/opt.h>
#include <kernel/init.h>
#include <kernel/kprintf.h>
#include <kernel/sched.h>
#include <kernel/memory.h>
#include <kernel/rootfs.h>
#include <kernel/syscall.h>
#include <kernel/panic.h>
#include <kernel/ktask.h>
#include <kernel/ipi.h>
#include <sys/compiler.h>
#include <sys/devicetree.h>
#include <sys/errno.h>
#include <sys/list.h>
#include <sys/mutex.h>
#include <sys/string.h>
#include <sys/types.h>


/* types */
typedef void (*modifier_t)(thread_t *this_t, void *payload);

typedef struct sched_queue_t{
	struct sched_queue_t *prev,
						 *next;

	thread_t *thread;
} sched_queue_t;

typedef struct{
	thread_t *this_t;
	modifier_t op;

	size_t size;
	uint8_t payload[] __align(CONFIG_KMALLOC_ALIGN);
} ipi_data_t;


/* local/static prototypes */
static void thread_transition(thread_t *this_t, thread_state_t state);
static int thread_core(thread_t *this_t);
static void thread_modify(thread_t *this_t, modifier_t op, void *payload, size_t size);
static void modify_state(thread_t *this_t, void *state);

#ifdef DEVTREE_ARCH_MULTI_CORE
static void ipi_hdlr(void *payload);
#endif // DEVTREE_ARCH_MULTI_CORE

static void cleanup(void *payload);


/* static variables */
static sched_queue_t *sched_queues[NTHREADSTATES] = { 0x0 };
static mutex_t sched_mtx = NOINT_MUTEX_INITIALISER();

// NOTE having valid entries is required for functions that use
//      sched_running() early on, e.g. nested mutexes
static thread_t kernel_threads[DEVTREE_ARCH_NCORES] = {
	{
		.tid = 0,
		.stack = &(page_t){ .phys_addr = 0x0 },
		.parent = &(process_t){ .name = "kernel" },
		.mtx = NOINT_MUTEX_INITIALISER(),
	}
};
static thread_t *running[DEVTREE_ARCH_NCORES] = { kernel_threads };


/* global functions */
void sched_yield(void){
	char dummy;


	if(!int_enabled())
		kpanic("interrupts are disabled, syscall not allowed\n");

	// actual thread switches are only performed in interrupt
	// service routines as it is required for syscalls
	(void)sc(SC_SCHEDYIELD, &dummy);
}

void sched_trigger(void){
	thread_t *this_t = 0x0;
	sched_queue_t *el;


	int_enable(false);
	mutex_lock(&sched_mtx);

	// NOTE The running thread might have already transitioned to a
	// 		different state, e.g. through the kernel signal mechanism
	// 		or a kill
	if(running[PIR]->state == RUNNING)
		thread_transition(running[PIR], READY);

	list_for_each(sched_queues[READY], el){
		if(el->thread->affinity & (0x1 << PIR)){
			this_t = el->thread;
			break;
		}
	}

	if(this_t == 0x0)
		kpanic("no ready thread\n");

	thread_transition(this_t, RUNNING);

	running[PIR] = this_t;

	mutex_unlock(&sched_mtx);

	// NOTE Do not re-enable interrupts to prevent interrupts from occurring until
	// 		the end of the current interrupt routine. This would otherwise cause
	// 		those thread contexts, located on the current thread's stack, being
	// 		pushed to the context stack of the thread that has just been made the
	// 		running one

	// TODO if the thread is not actually switched and the only ready thread
	// 		is a kernel thread suspend the core
	//		only suspend if sched_trigger() is called through yield, otherwise
	//		it might only be by interrupt and the thread might have some work to do
	//		!!! the suspension has to be outside this routine, otherwise a cascade
	//		of scheduler interrupts might occur leading to a stack overflow
}

thread_t *sched_running(void){
	if(running[PIR] == 0x0)
		kpanic("no running thread\n");

	return running[PIR];
}

thread_t *sched_running_nopanic(void){
	return running[PIR];
}

void sched_thread_transition(thread_t *this_t, thread_state_t state){
	// indirectly trigger a kernel panic, only the scheduler
	// is permitted to transition a thread to the RUNNING state
	if(state == RUNNING)
		state = CREATED;

	thread_modify(this_t, modify_state, &state, sizeof(thread_state_t));
}


/* local functions */
static int init(void){
	process_t *this_p;
	thread_t *this_t;


	/* init kernel process */
	this_p = kernel_threads[0].parent;

	// one thread per core
	for(size_t i=0; i<DEVTREE_ARCH_NCORES; i++){
		this_t = kernel_threads + i;

		// having the entry, stack and context points for kernel
		// threads set to zero is since:
		// 	- kernel threads are already running
		// 	- stack pages are only relevant for user space, since
		// 	  the kernel has a separate memory management
		// 	- kernel thread context is set automatically once the
		// 	  thread is interrupted for the first time
		this_t->tid = i;
		this_t->state = CREATED;
		this_t->priority = CONFIG_SCHED_PRIO_DEFAULT;
		this_t->affinity = (0x1 << i);
		this_t->parent = this_p;
		this_t->stack->phys_addr = KERNEL_STACK(i + 1);

		memcheck_stack_prime(this_t->stack);
		mutex_init(&this_t->mtx, MTX_NOINT);
		list_add_tail(this_p->threads, this_t);

		// add kernel threads to running queue
		thread_transition(this_t, READY);
		thread_transition(this_t, RUNNING);

		running[i] = this_t;
	}

	/* create init process */
	this_p = process_create(kopt.init_bin, kopt.init_type, "init", kopt.init_arg, fs_root);

	if(this_p == 0x0)
		goto err;

	// add first thread to ready queue
	thread_transition(this_p->threads, READY);

	/* create cleanup task */
	if(ktask_create(cleanup, 0x0, 0, 0x0, true) != 0)
		goto err;

	return 0;


err:
	/* NOTE cleanup in case of an error is not required, since the
	 * 		kernel will stop anyways if any of the init calls fails
	 */
	return -errno;
}

kernel_init(2, init);

/**
 * \brief	move thread between scheduler queues according to the following
 * 			state machine
 *
 * 			NOTE higher level APIs allow waiting threads to be killed, however
 * 				 the underyling usignal mechanism ensures that signals are only
 * 				 processes once the thread has left the kernel (entering the
 * 				 WAITING state is only possible within the kernel). Hence, it
 * 				 must never be the case that thread_transition() is called to
 * 				 move a thread from WAITING to DEAD.
 *
 *            WAITING
 *             |   A
 *             |   | sig wait
 *    sig_wake |   |
 *             |   |           exit
 *             |  RUNNING ------------\
 *             |  |A                  |
 *             |  || sched            |
 *             |  ||                  |
 *             V  V|       kill       V
 * CREATED --> READY --------------> DEAD
 *
 *
 * \pre	calls to thread_transition are protected through sched_lock
 */
static void thread_transition(thread_t *this_t, thread_state_t state){
	thread_state_t s = this_t->state;
	sched_queue_t *e;


	/* check for invalid state transition */
	if((state == CREATED)
	|| (s == DEAD)
	|| (state == RUNNING && s != READY)
	|| (state == WAITING && s != RUNNING)
	|| (state == DEAD && s == CREATED)
	|| (state == DEAD && s == WAITING)
	){
		kpanic("invalid scheduler transition %u -> %u\n", s, state);
	}

	/* perform transition */
	if(this_t->state == CREATED){
		e = kpalloc(sizeof(sched_queue_t));
		e->thread = this_t;
	}
	else{
		e = list_find(sched_queues[this_t->state], thread, this_t);

		if(e == 0x0)
			kpanic("thread not found in supposed sched queue %d\n", this_t->state);

		list_rm(sched_queues[this_t->state], e);
	}

	list_add_tail(sched_queues[state], e);
	this_t->state = state;
}

static int thread_core(thread_t *this_t){
	for(size_t core=0; core<DEVTREE_ARCH_NCORES; core++){
		if(running[core] == this_t)
			return core;
	}

	return -1;
}

static void thread_modify(thread_t *this_t, modifier_t op, void *payload, size_t size){
#ifdef DEVTREE_ARCH_MULTI_CORE
	int core;
	ipi_data_t *ipi;
	char blob[sizeof(ipi_data_t) + size];
#endif // DEVTREE_ARCH_MULTI_CORE


	mutex_lock(&sched_mtx);

#ifdef DEVTREE_ARCH_MULTI_CORE
	/* identify core for current thread */
	core = thread_core(this_t);

	/* trigger modification */
	if(this_t->state == RUNNING && core != PIR){
		ipi = (ipi_data_t*)blob;

		ipi->this_t = this_t;
		ipi->op = op;
		ipi->size = size;
		memcpy(ipi->payload, payload, size);

		if(ipi_send(core, ipi_hdlr, ipi, sizeof(ipi_data_t) + size) != 0)
			kpanic("trigger ipi failed \"%s\"\n", strerror(errno));
	}
	else
#endif // DEVTREE_ARCH_MULTI_CORE
		op(this_t, payload);

	mutex_unlock(&sched_mtx);
}

static void modify_state(thread_t *this_t, void *state){
	thread_transition(this_t, *((thread_state_t*)state));
}

#ifdef DEVTREE_ARCH_MULTI_CORE
static void ipi_hdlr(void *payload){
	ipi_data_t *p = (ipi_data_t*)payload;


	thread_modify(p->this_t, p->op, p->payload, p->size);
}
#endif // DEVTREE_ARCH_MULTI_CORE

/**
 * \brief	recurring task used to cleanup terminated threads
 * 			and processes
 */
static void cleanup(void *payload){
	sched_queue_t *e;
	process_t *this_p;
	thread_t *this_t;


	/* NOTE	use while rather than list_for_each to keep the critical
	 * 		section as short as possible, reducing the time of disabled
	 * 		interrupts
	 */
	while(1){
		/* remove thread scheduler queue entry */
		mutex_lock(&sched_mtx);

		e = list_first(sched_queues[DEAD]);

		if(e == 0x0)
			break;

		this_t = e->thread;

		list_rm(sched_queues[this_t->state], e);
		kfree(e);

		mutex_unlock(&sched_mtx);

		/* wait for thread being removed as running thread */
		while(thread_core(this_t) != -1)
			sched_yield();

		/* destroy thread and potentially parent process */
		this_p = this_t->parent;
		DEBUG("cleanup thread %s.%d\n", this_p->name, this_t->tid);
		thread_destroy(this_t);

		if(list_empty_safe(this_p->threads, &this_p->mtx)){
			DEBUG("cleanup process %s\n", this_p->name);
			process_destroy(this_p);
		}
	}

	mutex_unlock(&sched_mtx);
}
