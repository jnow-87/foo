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
#include <sys/types.h>
#include <sys/string.h>


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
	lnx_sigset_t blocked;


	memset(&blocked, 0x0, sizeof(lnx_sigset_t));

	// NOTE Ensure signals for higher priority interrupt cannot be interrupted by
	// 		signals of lower priority interrupts.
	//		Without this protection the signal handler for a lower priority interrupt
	//		can be called if the higher priority interrupt already completed the
	//		HWO_INT_RETURN hardware-op but did not yet return from the signal. Since
	//		lower priority signals shall by design be interruptible by higher priority
	//		ones, this could lead to a deadlock with the integration test framework
	//		awaiting the hardware-op acknowledgement for the higher priority interrupt
	//		while the kernel is unable to send it, since the signal handler for the
	//		higher priority interrupt cannot be called-
	for(size_t i=1; i<=X86_INT_PRIOS; i++){
		lnx_sigaction(CONFIG_TEST_INT_HW_SIG + X86_INT_PRIOS - i, int_hdlr, &blocked);
		lnx_sigaddset(&blocked, CONFIG_TEST_INT_HW_SIG + X86_INT_PRIOS - i);
	}

	return 0;
}

platform_init(0, init);

static void int_hdlr(int sig){
	x86_hw_op_t op;
	thread_ctx_t *ctx;
	thread_t const *this_t;


	/* preamble */
	// acknowledge request
	x86_hw_op_read(&op);
	op.retval = 0;
	x86_hw_op_read_writeback(&op);

	if(op.num != HWO_INT_TRIGGER)
		LNX_EEXIT("[%u] invalid hardware-op %d to kernel\n", op.seq, op.num);

	int_op = &op;
	this_t = sched_running();

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
	int_khdlr(op.int_ctrl.num);

	/* epilogue */
	this_t = sched_running();
	int_op = 0x0;

	// pop thread context
	ctx = thread_ctx_pop();

	// CTX_UNKNOWN is used by the x86 implementation to mark
	// the initial init process ctx, since it resides on the
	// process stack it must not be freed
	if(ctx->type != CTX_UNKNOWN)
		kfree(ctx);

	// signal completion
	LNX_DEBUG("[%u] return to %s(pid = %u, tid = %u)\n",
		op.seq,
		this_t->parent->name,
		this_t->parent->pid,
		this_t->tid
	);

	op.num = HWO_INT_RETURN;
	op.int_return.num = op.int_ctrl.num;
	op.int_return.to = (this_t->parent->pid == 0) ? PRIV_KERNEL : PRIV_USER;
	op.int_return.tid = this_t->tid;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);
}
