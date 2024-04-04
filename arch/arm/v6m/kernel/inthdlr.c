/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/v6m.h>
#include <kernel/init.h>
#include <kernel/interrupt.h>
#include <kernel/panic.h>
#include <kernel/thread.h>
#include <sys/types.h>


/* external variables */
extern void (*__kernel_start[])(void);
extern void (*__kernel_end[])(void);


/* local/static prototypes */
static void hfault_hdlr(int_num_t num, void *payload);
static thread_ctx_type_t ctx_type(thread_ctx_t *ctx);


/* global functions */
thread_ctx_t *av6m_int_hdlr(thread_ctx_t *ctx){
	uint16_t num;


	ctx->this = ctx;
	ctx->type = ctx_type(ctx);

	thread_ctx_push(ctx);

	num = ppb_read(ICSR) & 0x1ff;

	// pendsv is used to trigger syscalls
	// from within the kernel
	if(num == INT_PENDSV)
		num = INT_SVCALL;

	int_khdlr(num);

	return thread_ctx_pop();
}

void av6m_inval_hdlr(void){
	kpanic("invalid interrupt %u\n", ppb_read(ICSR) & 0x1ff);
}


/* local functions */
static int init(void){
	return int_register(INT_HARDFAULT, hfault_hdlr, 0x0);
}

platform_init(1, first, init);

static void hfault_hdlr(int_num_t num, void *payload){
	kpanic("hard fault\n");
}

static thread_ctx_type_t ctx_type(thread_ctx_t *ctx){
	if(ctx->ret_addr >= (void*)__kernel_start && ctx->ret_addr <= (void*)__kernel_end)
		return CTX_KERNEL;

	return CTX_USER;
}
