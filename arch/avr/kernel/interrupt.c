/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <arch/avr/register.h>
#include <arch/avr/thread.h>
#include <kernel/interrupt.h>
#include <kernel/sched.h>
#include <kernel/thread.h>
#include <kernel/panic.h>
#include <sys/types.h>
#include <sys/compiler.h>
#include <sys/register.h>
#include <sys/stack.h>


/* external prototypes */
extern void int_vectors_lvl1(void);


/* external variables */
extern void (*__kernel_start_wa[])(void);
extern void (*__kernel_end_wa[])(void);


/* local/static prototypes */
static thread_ctx_type_t ctx_type(thread_ctx_t *ctx);


/* global functions */
int_type_t avr_int_enable(int_type_t mask){
	int_type_t s;


	s = (mreg_r(SREG) & (0x1 << SREG_I)) ? INT_GLOBAL : INT_NONE;

	if(mask)	asm volatile("sei");
	else		asm volatile("cli");

	return s;
}

int_type_t avr_int_enabled(void){
	if(mreg_r(SREG) & (0x1 << SREG_I))
		return INT_GLOBAL;

	return INT_NONE;
}

struct thread_ctx_t *avr_int_hdlr(struct thread_ctx_t *ctx){
	int_num_t num;


	/* save context */
	ctx->this = ctx;
	ctx->type = ctx_type(ctx);

	thread_ctx_push(ctx);

	/* call handler */
	// compute interrupt number
	// 	INT_VEC lengths are divided by 2 since the flash is word-addressed
	num = (lo8(ctx->int_vec_addr) << 8 | hi8(ctx->int_vec_addr));
	num = (num - (unsigned int)int_vectors_lvl1 - XCALL_LEN / 2) / (INT_VEC1_LEN / 2);

	int_khdlr(num);

	/* restore context */
	return thread_ctx_pop();
}

void avr_int_warm_reset_hdlr(void){
	kpanic("call reset handler without actual MCU reset\n");
}


/* local functions */
static thread_ctx_type_t ctx_type(thread_ctx_t *ctx){
	void *ret_addr;


	ret_addr = (void*)((lo8(ctx->ret_addr) << 8) | hi8(ctx->ret_addr));

	if(ret_addr >= (void*)__kernel_start_wa && ret_addr <= (void*)__kernel_end_wa)
		return CTX_KERNEL;

	return CTX_USER;
}
