/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
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
static bool int_en = false;
static x86_hw_op_t *int_op = 0x0;
static bool prevent_sched_transition = false;


/* global functions */
bool x86_int_enable(bool en){
	bool s = int_en;


	if(int_en != en){
		int_en = en;
		x86_hw_int_set(en);
	}

	return s;
}

bool x86_int_enabled(void){
	return int_en;
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
		lnx_sigaction(CONFIG_X86EMU_HW_SIG + X86_INT_PRIOS - i, int_hdlr, &blocked);
		lnx_sigaddset(&blocked, CONFIG_X86EMU_HW_SIG + X86_INT_PRIOS - i);
	}

	return 0;
}

platform_init(1, first, init);

static void int_hdlr(int sig){
	x86_hw_op_t op;
	thread_ctx_t *ctx;
	thread_t *this_t;


	/* preamble */
	int_en = false;

	// acknowledge request
	x86_hw_op_read(&op);
	op.retval = 0;
	x86_hw_op_read_writeback(&op);

	if(op.num != HWO_INT_TRIGGER)
		LNX_EEXIT("[%u] invalid hardware-op %d to kernel\n", op.seq, op.num);

	int_op = &op;
	this_t = sched_running();

	// ensure the active thread is not changed if
	// the interrupt occured while serving a syscall
	if(op.int_ctrl.num == INT_SYSCALL && this_t->parent->pid != 0)
		prevent_sched_transition = true;

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

	if(op.int_ctrl.num == INT_SYSCALL)
		x86_sc_epilogue(this_t);

	if(prevent_sched_transition && this_t->parent != sched_running()->parent)
		x86_sched_force(this_t);

	/* epilogue */
	this_t = sched_running();
	int_op = 0x0;

	// pop thread context
	ctx = thread_ctx_pop();

	// issue usignal
	if(ctx->entry != 0x0 && ctx->entry == this_t->parent->sig_hdlr)
		x86_hw_usignal_send(ctx->arg);

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

	x86_hw_int_return(op.int_ctrl.num, (this_t->parent->pid == 0) ? PRIV_KERNEL : PRIV_USER, this_t->tid);

	if(op.int_return.num == INT_SYSCALL && this_t->parent->pid != 0)
		prevent_sched_transition = false;

	int_en = true;
}
