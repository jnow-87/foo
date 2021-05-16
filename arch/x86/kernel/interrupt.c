/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/x86/linux.h>
#include <arch/x86/hardware.h>
#include <kernel/init.h>
#include <kernel/interrupt.h>
#include <kernel/memory.h>
#include <kernel/thread.h>
#include <kernel/sched.h>
#include <sys/stack.h>



/* local/static prototypes */
static void int_hdlr(int sig);


/* static variables */
static x86_hw_op_t *int_op = 0x0;


/* global functions */
int_type_t x86_int_enabled(void){
	x86_hw_op_t op;


	op.num = HWO_INT_STATE;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);

	return (op.int_ctrl.en ? INT_GLOBAL : INT_NONE);
}

int_type_t x86_int_enable(int_type_t mask){
	x86_hw_op_t op;
	int_type_t s;


	s = x86_int_enabled();

	op.num = HWO_INT_SET;
	op.int_ctrl.en = (mask == INT_NONE ? 0 : 1);

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);

	return s;
}

x86_hw_op_t *x86_int_op(void){
	if(int_op == 0x0)
		LNX_EEXIT("no ongoing hardware-op\n");

	return int_op;
}


/* local functions */
static int init(void){
	lnx_sigset(CONFIG_TEST_INT_HW_SIG, int_hdlr);

	return E_OK;
}

platform_init(0, init);

static void int_hdlr(int sig){
	x86_hw_op_t op;
	thread_ctx_t *ctx;
	thread_t const *this_t;


	/* preamble */
	this_t = sched_running();

	// get interrupt data
	x86_hw_op_read(&op);
	op.retval = 0;
	x86_hw_op_read_writeback(&op);

	if(int_op)
		LNX_EEXIT("[%u] hardware-op in progress\n", op.seq);

	int_op = &op;

	if(op.num != HWO_INT_TRIGGER)
		LNX_EEXIT("[%u] invalid hardware-op %d to kernel\n", op.seq, op.num);

	LNX_DEBUG("[%u] %s interrupt on %s(pid = %u, tid = %u)\n",
		op.seq,
		X86_INT_NAME(op.int_ctrl.num),
		this_t->parent->name,
		this_t->parent->pid,
		this_t->tid
	);

	// push thread context
	ctx = kpalloc(sizeof(thread_ctx_t));

	ctx->this = ctx;
	ctx->type = (this_t->parent->pid == 0) ? CTX_KERNEL : CTX_USER;

	thread_ctx_push(ctx);

	/* handle interrupt */
	int_call(op.int_ctrl.num);

	/* epilogue */
	this_t = sched_running();

	// pop thread context
	ctx = thread_ctx_pop();

	// CTX_UNKNOWN is used by the x86 implementation to mark
	// the initial init process ctx, since it resides on the
	// process stack it must not be freed
	if(ctx->type != CTX_UNKNOWN)
		kfree(ctx);

	// reset interrupt op
	int_op = 0x0;

	// signal completion
	LNX_DEBUG("[%u] return to %s(pid = %u, tid = %u)\n",
		op.seq,
		this_t->parent->name,
		this_t->parent->pid,
		this_t->tid
	);

	op.num = HWO_INT_RETURN;
	op.int_return.to = (this_t->parent->pid == 0) ? PRIV_KERNEL : PRIV_USER;
	op.int_return.tid = this_t->tid;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);
}
