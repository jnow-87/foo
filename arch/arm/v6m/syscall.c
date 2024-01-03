#include <arch/arm/v6m.h>
#include <kernel/thread.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/syscall.h>
#include <sys/stack.h>


/* global functions */
int av6m_sc(sc_num_t num, void *param, size_t psize){
	sc_t volatile sc;


	/* prepare paramter */
	sc.num = num;
	sc.param = param;
	sc.size = psize;
	sc.errnum = E_UNKNOWN;

	/* trigger syscall */
#ifdef BUILD_KERNEL
	// Use the pendsv exception to trigger kernel syscalls since
	// the kernel uses them, for instance, to implement kernel
	// signals, which can be used during user space syscalls.
	// Hence, the svcall exception cannot be used, since the
	// exception cannot preempt itself and in case of svcall a
	// hard fault would even be triggered.
	asm volatile(
		"mov	r0, %[sc]\n"
		"ldr	r1, =%[icsr]\n"
		"str	%[trigger], [r1]\n"
		"isb\n"
		:
		: [sc] "r" (&sc),
		  [trigger] "r" (ICSR | (0x1 << ICSR_PENDSVSET)),
		  [icsr] "i" (&ICSR)
		: "r0", "r1", "memory"
	);
#else
	asm volatile(
		"mov	r0, %[sc]\n"
		"svc	0x0\n"
		:
		: [sc] "r" (&sc)
		: "r0"
	);
#endif // BUILD_KERNEL

	if(sc.errnum)
		return_errno(sc.errnum);

	return 0;
}

sc_t *av6m_sc_arg(thread_t *this_t){
	thread_ctx_t *ctx;


	ctx = stack_top(this_t->ctx_stack);

	return (sc_t*)(ctx->gpr_0_3[0]);
}
