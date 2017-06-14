#include <config/config.h>
#include <arch/interrupt.h>
#include <kernel/init.h>
#include <kernel/opt.h>
#include <kernel/sched.h>
#include <kernel/process.h>
#include <kernel/kmem.h>
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
static errno_t sched_init(void);
static errno_t sched_tick(int_num_t num);
static errno_t sched_queue_add(sched_queue_t **queue, thread_t *this_t);


/* global variables */
thread_t *current_thread[CONFIG_NCORES];
process_t *process_table = 0;


/* static variables */
static sched_queue_t *queue_ready = 0,
					 *queue_waiting = 0;


/* global functions */
void sched_resched(void){
}


/* local functions */
static errno_t sched_init(void){
	unsigned int i;
	process_t *this_p;
	thread_t *this_t;
	sched_queue_t *queue_e;


	/* register scheduler interrupt */
	if(int_hdlr_register(CONFIG_SCHED_INT, sched_tick) != E_OK)
		goto err_0;

	/* create kernel process */
	this_p = kmalloc(sizeof(process_t));

	if(this_p == 0)
		goto_errno(err_0, E_NOMEM);

	memset(this_p, 0x0, sizeof(process_t));

	this_p->affinity = CONFIG_SCHED_AFFINITY_DEFAULT;
	this_p->priority = CONFIG_SCHED_PRIO_DEFAULT;

	list_add_tail(process_table, this_p);

	/* create kernel thread */
	// allocate one thread per core
	for(i=0; i<CONFIG_NCORES; i++){
		this_t = kmalloc(sizeof(thread_t));

		if(this_t == 0)
			goto_errno(err_1, E_NOMEM);

		memset(this_t, 0x0, sizeof(thread_t));

		this_t->tid = i;
		this_t->state = CREATED;
		this_t->priority = CONFIG_SCHED_PRIO_DEFAULT;
		this_t->affinity = (0x1 << i);
		this_t->stack = (void*)KERNEL_STACK_CORE_BASE(i);
		this_t->parent = this_p;

		list_add_tail(this_p->threads, this_t);
		current_thread[i] = this_t;
	}

	// add kernel threads to ready queue
	list_for_each(this_p->threads, this_t){
		if(sched_queue_add(&queue_ready, this_t) != E_OK)
			goto err_2;
	}

	/* create init process */
	// create init processs
	this_p = process_create(kopt.init_bin, kopt.init_type, "init", kopt.init_arg, 0);

	if(this_p == 0)
		goto err_2;

	list_add_tail(process_table, this_p);

	// add first thread to ready queue
	if(sched_queue_add(&queue_ready, this_p->threads) != E_OK)
		goto err_2;

	return E_OK;

err_2:
	list_for_each(queue_ready, queue_e){
		list_rm(queue_ready, queue_e);
		kfree(queue_e);
	}

err_1:
	list_for_each(process_table, this_p){
		list_for_each(this_p->threads, this_t){
			list_rm(this_p->threads, this_t);
			kfree(this_t);
		}

		list_rm(process_table, this_p);
		kfree(this_p);
	}

err_0:
	return errno;
}

kernel_init(2, sched_init);

static errno_t sched_tick(int_num_t num){
	// TODO check for next thread
	// TODO switch thread or goto sleep
	return E_OK;
}

static errno_t sched_queue_add(sched_queue_t **queue, thread_t *this_t){
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
