/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <arch/interrupt.h>
#include <arch/core.h>
#include <arch/io.h>
#include <arch/avr/timer.h>
#include <arch/avr/iovfl.h>
#include <arch/avr/ctxsanity.h>
#include <kernel/kprintf.h>
#include <kernel/sched.h>
#include <kernel/panic.h>
#include <sys/errno.h>
#include <sys/stack.h>
#include <sys/register.h>
#include <sys/compiler.h>


/* external prototypes */
extern void int_vectors(void);


/* static variables */
static int_hdlr_t int_hdlr[NUM_INT] = { 0x0 };
static void *int_data[NUM_INT] = { 0x0 };


/* global functions */
int avr_int_register(int_num_t num, int_hdlr_t hdlr, void *data){
	if(num >= NUM_INT)
		return_errno(E_INVAL);

	if(int_hdlr[num] != 0x0)
		return_errno(E_INUSE);

	int_hdlr[num] = hdlr;
	int_data[num] = data;

	return E_OK;
}

void avr_int_release(int_num_t num){
	if(num >= NUM_INT)
		return;

	int_hdlr[num] = 0x0;
	int_data[num] = 0x0;
}

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

struct thread_ctx_t *avr_int_hdlr(struct thread_ctx_t *tc){
	uint8_t num;
	int sc;
	int stack_err;
	thread_ctx_t *tc_cmp;
	thread_t const *this_t_entry,
				   *this_t_exit;


	stack_err = 0;
	sc = -1;

	num = (lo8(tc->int_vec_addr) << 8 | hi8(tc->int_vec_addr));
	num = ((num - (unsigned int)int_vectors) / (INT_VEC_SIZE / 2)) - 1;

	if(num == NUM_HW_INT)
		sc = ctxsanity_sc_num();


	/* save thread context of active thread*/
	this_t_entry = sched_running();

	stack_err |= ctxsanity_check_stack_addr("entry", num, sc, tc, this_t_entry);
	stack_err |= ctxsanity_check_all_ctx("entry0", num, sc);

	stack_push(sched_running()->ctx_stack, tc);

	tc_cmp = kmalloc(sizeof(thread_ctx_t));
	memcpy(tc_cmp, tc, sizeof(thread_ctx_t));
	stack_push(sched_running()->ctx_stack_cmp, tc_cmp);

	/* compute interrupt number */
	// INT_VEC_SIZE / 2 is used since the flash is word-addressed
	num = (lo8(tc->int_vec_addr) << 8 | hi8(tc->int_vec_addr));
	num = ((num - (unsigned int)int_vectors) / (INT_VEC_SIZE / 2)) - 1;

	/* call interrupt handler */
	if(num >= NUM_INT || int_hdlr[num] == 0x0)
		kpanic(sched_running(), "unhandled or invalid interrupt %u", num);

	errno = E_OK;
	int_hdlr[num](num, int_data[num]);

	/* return context of active thread */
	BUILD_ASSERT(sizeof(errno_t) == 1);	// check sizeof thread_ctx_t::errno

	stack_err |= ctxsanity_check_all_ctx("exit0", num, sc);

	this_t_exit = sched_running();

	tc = stack_pop(((thread_t*)this_t_exit)->ctx_stack);
	tc_cmp = stack_pop(((thread_t*)this_t_exit)->ctx_stack_cmp);

	stack_err |= ctxsanity_check_stack_addr("exit", num, sc, tc, this_t_exit);
	stack_err |= ctxsanity_check_all_ctx("exit1", num, sc);
	stack_err |= ctxsanity_check_ctx("exit", num, sc, tc, tc_cmp, this_t_entry, this_t_exit);

	kfree(tc_cmp);

	if(stack_err){
		FATAL("thread context corruption detected, halting\n");
		while(1);
	}

	return tc;
}

void avr_int_warm_reset_hdlr(void){
	kpanic(0x0, "call reset handler without actual MCU reset");
}
