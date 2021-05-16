/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/list2.h>
#include <arch/x86/hardware.h>
#include <user/debug.h>
#include <hardware/hardware.h>
#include <brickos/brickos.h>
#include <brickos/child.h>


/* types */
typedef struct int_req_t{
	struct int_req_t *prev,
					 *next;

	int num;
	void *data;

	x86_priv_t src;
	unsigned int tid;
} int_req_t;


/* local/static prototypes */
static void req_enqueue(int_req_t *req);
static int_req_t *req_dequeue(void);
static int_req_t *req_lookup(void);

static void int_trigger(int_req_t *req);
static void int_return(x86_hw_op_src_t target);


/* static variables */
int_req_t *requests[3] = { 0x0 };
pthread_mutex_t request_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t request_sig = PTHREAD_COND_INITIALIZER;

pthread_mutex_t return_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t return_sig = PTHREAD_COND_INITIALIZER;


/* global functions */
void hw_int_process(void){
	int_req_t *req;


	req = req_dequeue();

	hw_state_lock();

	// ensure interrupts are only triggered for the appropriate
	// privilege level and thread in order to prevent confusing the
	// kernel scheduler by e.g. triggering a syscall from user space
	// while a kernel thread is active according to the scheduler
	if(req->src == PRIV_HARDWARE
	|| ((hw_state.privilege == PRIV_USER || req->src == hw_state.privilege) && hw_state.tid == req->tid)
	){
		while(!hw_state.int_enabled){
			hw_state_wait();
		}

		int_trigger(req);
		free(req);

		hw_state.stats.int_ack++;
	}
	else{
		req_enqueue(req);
		hw_state.stats.int_nack++;
	}

	hw_state_unlock();
}

void hw_int_request(int num, void *data, x86_priv_t src, unsigned int tid){
	int_req_t *req;


	req = malloc(sizeof(int_req_t));

	if(req == 0x0)
		EEXIT("out of memory\n");

	req->num = num;
	req->data = data;
	req->src = src;
	req->tid = tid;

	req_enqueue(req);
}

void hw_int_return(x86_hw_op_src_t target, unsigned int tid){
	int_return(target);

	hw_state.tid = tid;
}


/* local functions */
static void req_enqueue(int_req_t *req){
	pthread_mutex_lock(&request_mtx);

	__list2_add_tail(requests[hw_state.privilege], req, prev, next);
	pthread_cond_signal(&request_sig);

	pthread_mutex_unlock(&request_mtx);
}

static int_req_t *req_dequeue(void){
	int_req_t *req;


	pthread_mutex_lock(&request_mtx);

	while((req = req_lookup()) == 0x0){
		pthread_cond_wait(&request_sig, &request_mtx);
	}

	pthread_mutex_unlock(&request_mtx);

	return req;
}

static int_req_t *req_lookup(void){
	size_t i;
	int_req_t *req;
	x86_hw_op_src_t srcs[] = { PRIV_HARDWARE, hw_state.privilege };


	for(i=0; i<sizeof_array(srcs); i++){
		req = requests[srcs[i]];

		if(req != 0x0){
			__list2_rm(requests[srcs[i]], req, prev, next);
			break;
		}
	}

	return req;
}

static void int_trigger(int_req_t *req){
	x86_hw_op_t op;


	/* update hardware state */
	hw_state.int_enabled = false;
	hw_state.privilege = PRIV_KERNEL;

	if(req->num == INT_SYSCALL)
		hw_state.syscall_pending = true;

	/* interrupt kernel */
	pthread_mutex_lock(&return_mtx);

	op.num = HWO_INT_TRIGGER;
	op.int_ctrl.num = req->num;
	op.int_ctrl.data = req->data;

	child_lock(KERNEL);

	DEBUG(0, "trigger %s interrupt requested by %s\n", X86_INT_NAME(req->num), X86_PRIV_NAME(req->src));

	hw_op_write(&op, KERNEL);
	hw_op_write_writeback(&op, KERNEL);

	child_unlock(KERNEL);
	hw_state_unlock();

	/* wait for kernel to return */
	pthread_cond_wait(&return_sig, &return_mtx);
	pthread_mutex_unlock(&return_mtx);

	hw_state_lock();
}

static void int_return(x86_hw_op_src_t target){
	x86_hw_op_t op;


	/* restore hardware state */
	hw_state.int_enabled = true;
	hw_state.privilege = target;

	/* signal interrupt return */
	pthread_mutex_lock(&return_mtx);
	pthread_cond_signal(&return_sig);
	pthread_mutex_unlock(&return_mtx);

	/* signal return to user-space */
	if(target != PRIV_USER || !hw_state.syscall_pending)
		return;

	hw_state.syscall_pending = false;

	op.num = HWO_INT_RETURN;
	op.retval = 0;

	child_lock(APP);

	hw_op_write(&op, APP);
	hw_op_write_writeback(&op, APP);

	child_unlock(APP);

	if(op.retval != 0)
		EEXIT("int return to user failed with %d\n", op.retval);
}
