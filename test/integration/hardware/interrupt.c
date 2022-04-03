/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <pthread.h>
#include <arch/x86/hardware.h>
#include <sys/types.h>
#include <sys/list2.h>
#include <user/debug.h>
#include <hardware/hardware.h>
#include <brickos/brickos.h>
#include <brickos/child.h>


/* types */
typedef struct int_req_t{
	struct int_req_t *prev,
					 *next;

	size_t id;

	int num;
	bool pending;

	void *data;
	x86_priv_t src;
	unsigned int tid;

	bool synchronous;
	pthread_mutex_t mtx;
	pthread_cond_t sig;
} int_req_t;


/* local/static prototypes */
static int_req_t *req_lookup(void);
static void req_postpone(int_req_t *req);

static void int_trigger(int_req_t *req);


/* static variables */
static size_t req_id = 0;
static int_req_t *requests[X86_INT_PRIOS] = { 0x0 };
static pthread_mutex_t req_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t req_sig = PTHREAD_COND_INITIALIZER;

static size_t priorities[] = {
	0,	// INT_TIMER
	// to prevent issues with the active thread, scheduler
	// and syscall interrupts need to have the same priority
	1,	// INT_SCHED
	1,	// INT_SYSCALL
	0,	// INT_UART0
	0,	// INT_UART1
};


/* global functions */
void hw_int_process(void){
	int_req_t *req;


	req = req_lookup();

	hw_state_lock();

	while(!hw_state.int_enabled){
		hw_state_wait();
	}

	// ensure interrupts are only triggered for the appropriate
	// privilege level and thread in order to prevent confusing the
	// kernel scheduler by e.g. triggering a syscall from user space
	// while a kernel thread is active according to the scheduler
	if(req->src == PRIV_HARDWARE
	|| ((hw_state.privilege == PRIV_USER || req->src == hw_state.privilege) && hw_state.tid == req->tid)
	){
		int_trigger(req);
		hw_state.stats.int_ack++;
	}
	else{
		req_postpone(req);
		hw_state.stats.int_nack++;
	}

	hw_state_unlock();
}

void hw_int_request(int num, void *data, x86_priv_t src, unsigned int tid){
	int_req_t *req;


	/* allocate request */
	req = malloc(sizeof(int_req_t));

	if(req == 0x0)
		EEXIT("out of memory\n");

	req->num = num;
	req->pending = true;
	req->data = data;
	req->src = src;
	req->tid = tid;
	req->synchronous = (src == PRIV_HARDWARE) ? true : false;

	pthread_mutex_init(&req->mtx, 0x0);
	pthread_cond_init(&req->sig, 0x0);

	/* enqueue */
	if(req->synchronous)
		pthread_mutex_lock(&req->mtx);

	pthread_mutex_lock(&req_mtx);
	req->id = req_id++;
	DEBUG(0, "[%u] enqueue %s interrupt requested by %s\n", req->id, X86_INT_NAME(num), X86_PRIV_NAME(src));

	__list2_add_tail(requests[priorities[req->num]], req, prev, next);
	pthread_cond_signal(&req_sig);

	pthread_mutex_unlock(&req_mtx);

	/* wait for completion */
	if(req->synchronous){
		DEBUG(0, "[%u] waiting for %s interrupt completion\n", req->id, X86_INT_NAME(req->num));

		pthread_cond_wait(&req->sig, &req->mtx);
		pthread_mutex_unlock(&req->mtx);

		free(req);
	}
}

void hw_int_return(int num, x86_priv_t target, unsigned int tid){
	int_req_t *req;


	/* restore hardware state */
	hw_state.int_enabled = true;
	hw_state.ints_active--;
	hw_state.privilege = (hw_state.ints_active > 0) ? PRIV_KERNEL : target;
	hw_state.tid = tid;

	/* signal interrupt return */
	pthread_mutex_lock(&req_mtx);

	req = requests[priorities[num]];

	if(req == 0x0 || req->pending)
		EEXIT("invalid interrupt request list\n");

	__list2_rm(requests[priorities[num]], req, prev, next);

	pthread_cond_signal(&req_sig);
	pthread_mutex_unlock(&req_mtx);

	/* signal request completion */
	if(req->synchronous){
		DEBUG(0, "[%u] complete %s interrupt\n", req->id, X86_INT_NAME(req->num));

		pthread_mutex_lock(&req->mtx);
		pthread_cond_signal(&req->sig);
		pthread_mutex_unlock(&req->mtx);
	}
	else
		free(req);
}


/* local functions */
static int_req_t *req_lookup(void){
	size_t i;
	int_req_t *req;


	pthread_mutex_lock(&req_mtx);

	while(1){
		for(i=0; i<X86_INT_PRIOS; i++){
			if((req = requests[i]) != 0x0)
				break;
		}

		if(req != 0x0 && req->pending)
			break;

		pthread_cond_wait(&req_sig, &req_mtx);
	}

	pthread_mutex_unlock(&req_mtx);

	return req;
}

static void req_postpone(int_req_t *req){
	size_t prio;


	pthread_mutex_lock(&req_mtx);

	prio = priorities[req->num];
	__list2_rm(requests[prio], req, prev, next);
	__list2_add_tail(requests[prio], req, prev, next);

	pthread_mutex_unlock(&req_mtx);
}

static void int_trigger(int_req_t *req){
	x86_hw_op_t op;


	/* update hardware state */
	hw_state.int_enabled = false;
	hw_state.ints_active++;
	hw_state.privilege = PRIV_KERNEL;

	/* interrupt kernel */
	op.num = HWO_INT_TRIGGER;
	op.int_ctrl.num = req->num;
	op.int_ctrl.data = req->data;

	child_lock(KERNEL);

	DEBUG(0, "[%u] trigger %s interrupt requested by %s\n", req->id, X86_INT_NAME(req->num), X86_PRIV_NAME(req->src));

	hw_op_write_sig(&op, KERNEL, CONFIG_TEST_INT_HW_SIG + priorities[req->num]);
	hw_op_write_writeback(&op, KERNEL);

	child_unlock(KERNEL);

	req->pending = false;
}
