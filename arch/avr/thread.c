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
thread_context_t *avr_thread_context_init(thread_t *this_t, void *proc_entry, void *arg){
	thread_context_t *ctx;


	/* set thread context */
	ctx = (thread_context_t*)(this_t->stack->phys_addr + CONFIG_KERNEL_STACK_SIZE - sizeof(thread_context_t));

	/* init thread context */
	memset(ctx, 0x0, sizeof(thread_context_t));

	// set status and control registers
	ctx->sreg = mreg_r(SREG);
	ctx->mcusr = mreg_r(MCUSR);

	// init process start address
	if(proc_entry == (void*)CONFIG_INIT_BINARY)
		proc_entry = (void*)(((unsigned int)proc_entry) / 2);

	ctx->ret_addr = (void*)((lo8(proc_entry) << 8) | hi8(proc_entry));

	// set _start argument0: entry point
	ctx->gpr[24] = lo8(this_t->entry);
	ctx->gpr[25] = hi8(this_t->entry);

	if(this_t->entry == (void*)CONFIG_INIT_BINARY){
		ctx->gpr[24] = hi8(ctx->ret_addr);
		ctx->gpr[25] = lo8(ctx->ret_addr);
	}

	// set _start  argument1: thread argument
	ctx->gpr[22] = lo8(arg);
	ctx->gpr[23] = hi8(arg);


	return ctx;
}
