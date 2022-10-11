/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <pthread.h>
#include <arch/x86/hardware.h>
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
static int event_syscall_return(x86_hw_op_t *op);
static int event_copy_from_user(x86_hw_op_t *op);
static int event_copy_to_user(x86_hw_op_t *op);
static int event_uart_config(x86_hw_op_t *op);
static int event_display_config(x86_hw_op_t *op);
static int event_setup(x86_hw_op_t *op);
static int event_inval(x86_hw_op_t *op);

static int copy_op(child_t *tgt, child_t *src, x86_hw_op_t *op);


/* static variables */
static size_t events[2] = { 0 };
static pthread_mutex_t event_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t event_sig = PTHREAD_COND_INITIALIZER;

static ops_cfg_t hw_ops[] = {
	{ .name = "exit",			.hdlr = event_exit },
	{ .name = "int_trigger",	.hdlr = event_int_trigger },
	{ .name = "int_return",		.hdlr = event_int_return },
	{ .name = "int_set",		.hdlr = event_int_set },
	{ .name = "int_state",		.hdlr = event_int_state },
	{ .name = "syscall_return",	.hdlr = event_syscall_return },
	{ .name = "copy_from_user",	.hdlr = event_copy_from_user },
	{ .name = "copy_to_user",	.hdlr = event_copy_to_user },
	{ .name = "uart_config",	.hdlr = event_uart_config },
	{ .name = "display_config",	.hdlr = event_display_config },
	{ .name = "setup",			.hdlr = event_setup },
	{ .name = "invalid",		.hdlr = event_inval },
};


/* global functions */
void hw_op_write(x86_hw_op_t *op, child_t *tgt){
	hw_op_write_sig(op, tgt, CONFIG_TEST_INT_HW_SIG);
}

void hw_op_write_sig(x86_hw_op_t *op, child_t *tgt, int sig){
	static unsigned int seq_num[2] = { 0 };


	op->src = PRIV_HARDWARE;

	child_signal(tgt, sig);

	child_read(tgt, 0, &op->seq, sizeof(op->seq));
	CHECK_SEQ_NUM(op->seq, seq_num[(tgt == KERNEL) ? PRIV_KERNEL : PRIV_USER]++);

	child_write(tgt, 0, op, sizeof(*op));
}

void hw_op_write_writeback(x86_hw_op_t *op, child_t *tgt){
	unsigned int seq_num = op->seq;


	child_read(tgt, 0, op, sizeof(*op));
	CHECK_SEQ_NUM(op->seq, seq_num);

	child_write(tgt, 0, &op->seq, sizeof(op->seq));

	if(op->retval != 0)
		EEXIT("[%u] hardware-op %d returned failure %d\n", op->seq, op->num, op->retval);
}

void hw_op_read(x86_hw_op_t *op, child_t *src){
	static unsigned int seq_num[2] = { 0 };
	x86_priv_t idx = (src == KERNEL) ? PRIV_KERNEL : PRIV_USER;


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
	x86_hw_op_t op;
	child_t *src,
			*op_src;


	BUILD_ASSERT(sizeof_array(hw_ops) == HWO_NOPS + 1);

	src = hw_event_dequeue();

	hw_state_lock();
	child_lock(src);

	hw_op_read(&op, src);

	// ensure hardware events are only processed for the appropriate
	// privilege level and thread in order to prevent confusing the
	// kernel scheduler by e.g. triggering a syscall from user space
	// while a kernel thread is active according to the scheduler
	if((hw_state.privilege == (src == KERNEL ? PRIV_KERNEL : PRIV_USER))
	&& (hw_state.tid == op.tid || src == KERNEL)
	){
		DEBUG(2, "[%u] %s(src = %s, tid = %u, num = %d)\n",
			op.seq,
			hw_ops[op.num].name,
			src->name,
			op.tid,
			op.num
		);

		hw_op_read_ack(src, 1);

		if(op.num >= HWO_NOPS)
			EEXIT("  [%u] invalid hardware-op %d from %s\n", op.seq, op.num, src->name);

		if(op.src != PRIV_KERNEL && op.src != PRIV_USER)
			EEXIT("  [%u] invalid hardware-op src: %d\n", op.seq, op.src);

		op_src = (op.src == PRIV_KERNEL) ? KERNEL : APP;

		if(src != op_src)
			EEXIT("  [%u] hardware-op src mismatch: have %s expect %s\n", op.seq, op_src->name, src->name);

		op.retval = hw_ops[op.num].hdlr(&op);

		hw_op_read_writeback(&op, src);

		DEBUG(2, "  [%u] status: %s\n", op.seq, (op.retval == 0 ? "ok" : "error"));
		hw_state.stats.event_ack++;
	}
	else{
		hw_op_read_ack(src, 0);
		hw_state.stats.event_nack++;

		hw_event_enqueue(src);
	}

	child_unlock(src);
	hw_state_unlock();
}

void hw_event_enqueue(child_t *src){
	pthread_mutex_lock(&event_mtx);

	if(++events[src == KERNEL ? 0 : 1] == 0)
		EEXIT("event overflow\n");

	pthread_cond_signal(&event_sig);
	pthread_mutex_unlock(&event_mtx);
}

child_t *hw_event_dequeue(void){
	child_t *src;


	pthread_mutex_lock(&event_mtx);

	while(1){
		if(events[hw_state.privilege] != 0){
			events[hw_state.privilege]--;
			src = (hw_state.privilege == PRIV_KERNEL) ? KERNEL : APP;

			break;
		}

		pthread_cond_wait(&event_sig, &event_mtx);
	}

	pthread_mutex_unlock(&event_mtx);

	return src;
}


/* local functions */
static int event_exit(x86_hw_op_t *op){
	DEBUG(0, "  [%u] exit code: %d\n", op->seq, op->exit.retval);

	if(op->exit.retval != 0){
		ERROR("unexpected exit from %s, exit code %d\n",
			(op->src == PRIV_KERNEL) ? KERNEL->name : APP->name,
			op->exit.retval
		);
	}

	exit(op->exit.retval);
}

static int event_int_trigger(x86_hw_op_t *op){
	hw_int_request(op->int_ctrl.num, op->int_ctrl.payload, op->src, op->tid);

	return 0;
}

static int event_int_return(x86_hw_op_t *op){
	if(op->src != PRIV_KERNEL)
		EEXIT("int return only supposed to be triggered by kernel\n");

	DEBUG(0, "  [%u] return to %s space, tid %u\n",
		op->seq,
		X86_PRIV_NAME(op->int_return.to),
		op->int_return.tid
	);

	hw_int_return(op->int_return.num, op->int_return.to, op->int_return.tid);
	pthread_cond_signal(&event_sig);

	return 0;
}

static int event_int_set(x86_hw_op_t *op){
	DEBUG(2, "  [%u] int state: %d\n", op->seq, op->int_ctrl.en);

	if(hw_state.int_enabled != op->int_ctrl.en)
		DEBUG(0, "  [%u] change int state: %d\n", op->seq, op->int_ctrl.en);

	hw_state.int_enabled = op->int_ctrl.en;

	return op->int_ctrl.en == hw_state.int_enabled ? 0 : -1;
}

static int event_int_state(x86_hw_op_t *op){
	op->int_ctrl.en = hw_state.int_enabled;
	DEBUG(2, "  [%u] int state: %d\n", op->seq, op->int_ctrl.en);

	return 0;
}

static int event_syscall_return(x86_hw_op_t *op){
	x86_hw_op_t app_op = *op;


	if(app_op.src != PRIV_KERNEL)
		EEXIT("syscall return only supposed to be triggered by kernel\n");

	child_lock(APP);

	hw_op_write(&app_op, APP);
	hw_op_write_writeback(&app_op, APP);

	child_unlock(APP);

	if(app_op.retval != 0)
		EEXIT("syscall return failed with %d\n", app_op.retval);

	return 0;
}

static int event_copy_from_user(x86_hw_op_t *op){
	return copy_op(KERNEL, APP, op);
}

static int event_copy_to_user(x86_hw_op_t *op){
	return copy_op(APP, KERNEL, op);
}

static int event_uart_config(x86_hw_op_t *op){
	return uart_configure(op->uart.path, op->uart.int_num, &op->uart.cfg);
}

static int event_display_config(x86_hw_op_t *op){
	return display_configure(op->display.shm_id, op->display.scale, &op->display.cfg);
}

static int event_setup(x86_hw_op_t *op){
	x86_hw_op_t app_op = *op;


	child_lock(APP);

	hw_op_write(&app_op, APP);
	hw_op_write_writeback(&app_op, APP);

	child_unlock(APP);

	if(app_op.retval != 0)
		EEXIT("setup failed with %d\n", app_op.retval);

	return 0;
}

static int event_inval(x86_hw_op_t *op){
	DEBUG(0, "  [%u] invalid\n", op->seq);
	return -1;
}

static int copy_op(child_t *tgt, child_t *src, x86_hw_op_t *op){
	x86_hw_op_t app_op = *op;


	if(op->src != PRIV_KERNEL)
		EEXIT("copy from/to user only supposed to be triggered by kernel\n");

	child_lock(APP);

	hw_op_write(&app_op, APP);
	child_fwd(tgt, src, 0, op->copy.n);
	hw_op_write_writeback(&app_op, APP);

	child_unlock(APP);

	return app_op.retval;
}
