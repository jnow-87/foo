/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <arch/interrupt.h>

#ifdef BUILD_KERNEL
#include <kernel/init.h>
#include <kernel/syscall.h>
#include <kernel/sched.h>
#endif // BUILD_KERNEL

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/register.h>
#include <sys/compiler.h>
#include <sys/devicetree.h>


/* macros */
#define INT_VEC_SC		(DEVTREE_KERNEL_FLASH_BASE + INT_VEC_SIZE * NUM_HW_INT)
#define SYSCALL(addr)	asm volatile("call " STRGIFY(addr));


/* global functions */
int avr_sc(sc_t num, void *param, size_t psize){
	sc_arg_t volatile arg;


	/* clear interrupts */
	asm volatile("cli");

	/* prepare paramter */
	arg.num = num;
	arg.param = param;
	arg.size = psize;

	// copy address to GPIO registers 0/1
	mreg_w(GPIOR0, lo8(&arg));
	mreg_w(GPIOR1, hi8(&arg));

	/* trigger syscall */
	SYSCALL(INT_VEC_SC);

	BUILD_ASSERT(sizeof(errno_t) == 1);

	if(mreg_r(GPIOR0))
		return_errno(mreg_r(GPIOR0));
	return E_OK;
}


/* local functions */
#ifdef BUILD_KERNEL
static void sc_hdlr(int_num_t num, void *data){
	sc_arg_t *arg;
	thread_ctx_t *ctx;


	/* get address from GPIO registers 0/1 */
	arg = (sc_arg_t*)(mreg_r(GPIOR0) | (mreg_r(GPIOR1) << 8));

	/* get context early as the running thread might change during the call */
	ctx = sched_running()->ctx_stack;

	/* call kernel syscall handler */
	ksc_hdlr(arg->num, arg->param, arg->size);

	BUILD_ASSERT(sizeof(errno_t) == 1);

	/* set errno */
	ctx->gpior[0] = errno;
}
#endif // BUILD_KERNEL

#ifdef BUILD_KERNEL
static int init(void){
	return int_register(NUM_HW_INT, sc_hdlr, 0x0);
}

platform_init(0, init);
#endif // BUILD_KERNEL
