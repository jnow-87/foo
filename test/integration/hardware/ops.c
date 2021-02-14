/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <pthread.h>
#include <include/arch/x86/hardware.h>
#include <include/sys/list2.h>
#include <user/debug.h>
#include <hardware/hardware.h>
#include <brickos/child.h>
#include <brickos/brickos.h>


/* macros */
#define CHECK_SEQ_NUM(num, ref){ \
	typeof(num) _num = num; \
	typeof(ref) _ref = ref; \
	\
	\
	if(_num != _ref) \
		EEXIT("sequence number mismatch: %u expected %u\n", _num, _ref); \
}


/* types */
typedef struct event_t{
	struct event_t *prev,
				   *next;

	child_t *src;
} event_t;

typedef struct{
	char const *name;
	int (*hdlr)(x86_hw_op_t *op);
} ops_cfg_t;


/* local/static prototypes */
static int event_exit(x86_hw_op_t *op);

static int event_int_trigger(x86_hw_op_t *op);
static int event_int_return(x86_hw_op_t *op);

static int event_int_set(x86_hw_op_t *op);
static int event_int_state(x86_hw_op_t *op);

static int event_copy_from_user(x86_hw_op_t *op);
static int event_copy_to_user(x86_hw_op_t *op);

static int event_inval(x86_hw_op_t *op);

static int copy_op(child_t *tgt, child_t *src, x86_hw_op_t *op);


/* static variables */
static event_t *event_lst = 0x0;
static pthread_mutex_t event_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t event_sig = PTHREAD_COND_INITIALIZER;

static ops_cfg_t hw_ops[] = {
	{ .name = "exit",			.hdlr = event_exit },
	{ .name = "int trigger",	.hdlr = event_int_trigger },
	{ .name = "int return",		.hdlr = event_int_return },
	{ .name = "int set",		.hdlr = event_int_set },
	{ .name = "int state",		.hdlr = event_int_state },
	{ .name = "copy from user",	.hdlr = event_copy_from_user },
	{ .name = "copy to user",	.hdlr = event_copy_to_user },
	{ .name = "invalid",		.hdlr = event_inval },
};


/* global functions */
void hw_op_write(x86_hw_op_t *op, child_t *tgt){
	static unsigned int seq_num[2] = { 0 };


	op->src = HWS_HARDWARE;

	child_signal(tgt, CONFIG_TEST_INT_DATA_SIG);

	child_read(tgt, 0, &op->seq, sizeof(op->seq));
	CHECK_SEQ_NUM(op->seq, seq_num[(tgt == KERNEL) ? HWS_KERNEL : HWS_USER]++);

	child_write(tgt, 0, op, sizeof(*op));
}

void hw_op_write_writeback(x86_hw_op_t *op, child_t *tgt){
	unsigned int seq_num;


	seq_num = op->seq;

	child_read(tgt, 0, op, sizeof(*op));
	CHECK_SEQ_NUM(op->seq, seq_num);

	child_write(tgt, 0, &op->seq, sizeof(op->seq));

	if(op->retval != 0)
		EEXIT("[%u] hardware-op %d returned failure %d\n", op->seq, op->num, op->retval);
}

void hw_op_read(x86_hw_op_t *op, child_t *src){
	static unsigned int seq_num[2] = { 0 };
	x86_hw_op_src_t idx;


	idx = (src == KERNEL) ? HWS_KERNEL : HWS_USER;

	child_write(src, 0, seq_num + idx, sizeof(seq_num[0]));

	child_read(src, 0, op, sizeof(*op));
	CHECK_SEQ_NUM(op->seq, seq_num[idx]++);
}

void hw_op_read_ack(child_t *src, int ack){
	child_write(src, 0, &ack, sizeof(ack));
}

void hw_op_read_writeback(x86_hw_op_t *op, child_t *src){
	unsigned int seq_num;


	child_write(src, 0, op, sizeof(*op));

	child_read(src, 0, &seq_num, sizeof(seq_num));
	CHECK_SEQ_NUM(op->seq, seq_num);
}

void hw_event_process(void){
	bool handle;
	x86_hw_op_t op;
	child_t *src,
			*op_src;


	src = hw_event_dequeue();

	hw_state_lock();
	child_lock(src);

	hw_op_read(&op, src);

	// ensure hardware events are only processed for the appropriate
	// priviledge level in order to prevent confusing the kernel
	// scheduler by e.g. triggering a syscall from user space
	// while a kernel thread is active according to the scheduler
	handle = (hw_state.priviledge == (src == KERNEL ? HWS_KERNEL : HWS_USER));

	hw_op_read_ack(src, (handle ? 1 : 0));

	if(handle){
		DEBUG("[%u] hardware event from %s\n", op.seq, src->name);

		if(op.num >= HWO_NOPS)
			EEXIT("  [%u] invalid hardware-op %d from %s\n", op.seq, op.num, src->name);

		DEBUG("  [%u] event: %s (%d)\n", op.seq, hw_ops[op.num].name, op.num);

		if(op.src != HWS_KERNEL && op.src != HWS_USER)
			EEXIT("  [%u] invalid hardware-op src: %d\n", op.seq, op.src);

		op_src = (op.src == HWS_KERNEL) ? KERNEL : APP;

		if(src != op_src)
			EEXIT("  [%u] hardware-op src mismatch: have %s expect %s\n", op.seq, op_src->name, src->name);

		op.retval = hw_ops[op.num].hdlr(&op);

		hw_op_read_writeback(&op, src);

		DEBUG("  [%u] status: %s\n", op.seq, (op.retval == 0 ? "ok" : "error"));
	}
	else
		hw_event_enqueue(src);

	child_unlock(src);
	hw_state_unlock();
}

void hw_event_enqueue(child_t *src){
	event_t *e;


	e = malloc(sizeof(event_t));

	if(e == 0x0)
		EEXIT("out of memory\n");

	e->src = src;

	pthread_mutex_lock(&event_mtx);

	__list2_add_tail(event_lst, e, prev, next);
	pthread_cond_signal(&event_sig);

	pthread_mutex_unlock(&event_mtx);
}

child_t *hw_event_dequeue(void){
	event_t *e;
	child_t *src;


	pthread_mutex_lock(&event_mtx);

	while(1){
		e = event_lst;

		if(e != 0x0){
			__list2_rm(event_lst, e, prev, next);
			break;
		}

		pthread_cond_wait(&event_sig, &event_mtx);
	}

	pthread_mutex_unlock(&event_mtx);

	src = e->src;
	free(e);

	return src;
}


/* local functions */
static int event_exit(x86_hw_op_t *op){
	DEBUG("  [%u] exit code: %d\n", op->seq, op->exit.retval);

	if(op->exit.retval != 0){
		ERROR("unexpected exit from %s, exit code %d\n",
			(op->src == HWS_KERNEL) ? KERNEL->name : APP->name,
			op->exit.retval
		);
	}

	exit(op->exit.retval);
}

static int event_int_trigger(x86_hw_op_t *op){
	hw_int_request(op->int_ctrl.num, op->int_ctrl.data, op->src);

	return 0;
}

static int event_int_return(x86_hw_op_t *op){
	if(op->src != HWS_KERNEL)
		EEXIT("int return only supposed to be triggered by kernel\n");

	DEBUG("  [%u] return to %s space\n", op->seq, (op->int_ctrl.ret_to == HWS_USER) ? "user" : "kernel");

	hw_int_return(op->int_ctrl.ret_to);

	return 0;
}

static int event_int_set(x86_hw_op_t *op){
	hw_state.int_enabled = op->int_ctrl.en;
	DEBUG("  [%u] int state: %d\n", op->seq, op->int_ctrl.en);

	return op->int_ctrl.en == hw_state.int_enabled ? 0 : -1;
}

static int event_int_state(x86_hw_op_t *op){
	op->int_ctrl.en = hw_state.int_enabled;
	DEBUG("  [%u] int state: %d\n", op->seq, op->int_ctrl.en);

	return 0;
}

static int event_copy_from_user(x86_hw_op_t *op){
	return copy_op(KERNEL, APP, op);
}

static int event_copy_to_user(x86_hw_op_t *op){
	return copy_op(APP, KERNEL, op);
}

static int event_inval(x86_hw_op_t *op){
	DEBUG("  [%u] invalid\n", op->seq);
	return -1;
}

static int copy_op(child_t *tgt, child_t *src, x86_hw_op_t *op){
	x86_hw_op_t app_op;


	app_op = *op;

	if(op->src != HWS_KERNEL)
		EEXIT("copy from/to user only supposed to be triggered by kernel\n");

	child_lock(APP);

	hw_op_write(&app_op, APP);
	child_fwd(tgt, src, 0, op->copy.n);
	hw_op_write_writeback(&app_op, APP);

	child_unlock(APP);

	return app_op.retval;
}
