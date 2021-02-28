/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <arch/avr/register.h>
#include <kernel/thread.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/register.h>


/* macros */
#define AVR_ENTRY	(CONFIG_INIT_BINARY / 2)


/* external variables */
extern void (*__kernel_start_wa[])(void);
extern void (*__kernel_end_wa[])(void);


/* global functions */
void avr_thread_context_init(thread_ctx_t *ctx, struct thread_t *this_t, thread_entry_t entry, void *arg){
	memset(ctx, 0, sizeof(thread_ctx_t));

	ctx->mcusr = mreg_r(MCUSR);
	ctx->ret_addr = (void*)((lo8(AVR_ENTRY) << 8) | hi8(AVR_ENTRY));

	// set _start() arg0: entry point
	ctx->gpr[24] = lo8(entry);
	ctx->gpr[25] = hi8(entry);

	// set _start() arg1: thread argument
	ctx->gpr[22] = lo8(arg);
	ctx->gpr[23] = hi8(arg);
}

enum thread_ctx_type_t avr_thread_context_type(thread_ctx_t *ctx){
	void *ret_addr;


	ret_addr = (void*)((lo8(ctx->ret_addr) << 8) | hi8(ctx->ret_addr));

	if(ret_addr >= (void*)__kernel_start_wa && ret_addr <= (void*)__kernel_end_wa)
		return CTX_KERNEL;

	return CTX_USER;
}
