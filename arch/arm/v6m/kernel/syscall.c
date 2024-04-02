/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/v6m.h>
#include <kernel/thread.h>
#include <sys/errno.h>
#include <sys/stack.h>
#include <sys/syscall.h>
#include <sys/types.h>


/* global functions */
int av6m_sc(sc_num_t num, void *param, size_t psize){
	sc_t volatile sc;


	/* prepare paramter */
	sc.num = num;
	sc.param = param;
	sc.size = psize;
	sc.errnum = E_UNKNOWN;

	/* trigger syscall */
	// Use the pendsv exception to trigger kernel syscalls since
	// the kernel uses them, for instance, to implement kernel
	// signals, which can be used during user space syscalls.
	// The svcall exception cannot be used, since the exception
	// cannot preempt itself and in case of svcall a hard fault
	// would even be triggered.
	asm volatile(
		"mov	r0, %[sc]\n"
		"str	%[trigger], [%[icsr]]\n"
		"dsb\n"
		"isb\n"
		:
		: [sc] "r" (&sc),
		  [trigger] "r" (ppb_read(ICSR) | (0x1 << ICSR_PENDSVSET)),
		  [icsr] "r" (&ppb_read(ICSR))
		: "r0", "r1", "memory"
	);

	if(sc.errnum)
		return_errno(sc.errnum);

	return 0;
}

sc_t *av6m_sc_arg(thread_t *this_t){
	thread_ctx_t *ctx;


	ctx = stack_top(this_t->ctx_stack);

	return (sc_t*)(ctx->gpr_0_3[0]);
}
