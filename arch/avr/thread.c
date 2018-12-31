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


/* global functions */
void avr_thread_context_init(thread_context_t *ctx, struct thread_t *this_t, user_entry_t user_entry, thread_entry_t thread_entry, void *thread_arg){
	/* init thread context */
	memset(ctx, 0x0, sizeof(thread_context_t));

	// set status and control registers
	ctx->sreg = mreg_r(SREG);
	ctx->mcusr = mreg_r(MCUSR);

	// init process start address
	if(user_entry == (void*)CONFIG_INIT_BINARY)
		user_entry = (void*)(((unsigned int)user_entry) / 2);

	ctx->ret_addr = (void*)((lo8(user_entry) << 8) | hi8(user_entry));

	// set _start argument0: entry point
	ctx->gpr[24] = lo8(thread_entry);
	ctx->gpr[25] = hi8(thread_entry);

	if(thread_entry == (void*)CONFIG_INIT_BINARY){
		ctx->gpr[24] = hi8(ctx->ret_addr);
		ctx->gpr[25] = lo8(ctx->ret_addr);
	}

	// set _start  argument1: thread argument
	ctx->gpr[22] = lo8(thread_arg);
	ctx->gpr[23] = hi8(thread_arg);
}
