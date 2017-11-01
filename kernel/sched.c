#include <config/config.h>
#include <arch/interrupt.h>
#include <kernel/init.h>
#include <kernel/opt.h>
#include <kernel/sched.h>
#include <kernel/process.h>
#include <kernel/kmem.h>
#include <kernel/rootfs.h>
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
static int sched_queue_add(sched_queue_t **queue, thread_t *this_t);


/* global variables */
void *kernel_stack[CONFIG_NCORES] = { 0 };
process_t *process_table = 0;


/* static variables */
static thread_t *current_thread[CONFIG_NCORES] = { 0x0 };
static sched_queue_t *queue_ready = 0,
					 *queue_waiting = 0;


/* global functions */
thread_t const *sched_running(void){
	return current_thread[PIR];
}

void sched_ctx_enqueue(thread_context_t *ctx){
	current_thread[PIR]->ctx = ctx;
}

thread_context_t *sched_ctx_dequeue(void){
	return current_thread[PIR]->ctx;
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

void sched_resched(void){
}


/* local functions */
static int init(void){
	unsigned int i;
	process_t *this_p;
	thread_t *this_t;


	/* allocate kernel stack */
	for(i=0; i<CONFIG_NCORES; i++){
		kernel_stack[i] = kmalloc(CONFIG_KERNEL_STACK_SIZE);

		if(kernel_stack[i] == 0)
			goto_errno(err, E_NOMEM);

		kernel_stack[i] += CONFIG_KERNEL_STACK_SIZE - 1;
	}

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
		this_t->ctx = 0x0;		// kernel thread context is set automatically once
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

static int sched_queue_add(sched_queue_t **queue, thread_t *this_t){
	sched_queue_t *e;


	e = kmalloc(sizeof(sched_queue_t));

	if(e == 0)
		goto err;

	e->thread = this_t;
	list_add_tail(*queue, e);

	return E_OK;


err:
	return_errno(E_NOMEM);
}
