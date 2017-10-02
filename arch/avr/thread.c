#include <config/config.h>
#include <arch/arch.h>
#include <arch/avr/register.h>
#include <kernel/thread.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/register.h>


/* global functions */
thread_context_t *avr_thread_context_init(thread_t *this_t, void *arg){
	thread_context_t *ctx;


	/* set thread context */
	ctx = (thread_context_t*)(this_t->stack->phys_addr + CONFIG_THREAD_STACK_SIZE - sizeof(thread_context_t));

	/* init thread context */
	memset(ctx, 0x0, sizeof(thread_context_t));

	// set status and control registers
	ctx->sreg = mreg_r(SREG);
	ctx->mcusr = mreg_r(MCUSR);

	// set thread argument (void pointer)
	ctx->gpr[24] = lo8(arg);
	ctx->gpr[25] = hi8(arg);

	/* init thread entry/return address */
	ctx->ret_addr = (void*)(((lo8(this_t->entry) << 8) | hi8(this_t->entry)) / 2);

	return ctx;
}
