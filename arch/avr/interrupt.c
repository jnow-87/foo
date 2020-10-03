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
#include <kernel/panic.h>
#include <sys/types.h>
#include <sys/compiler.h>
#include <sys/register.h>
#include <sys/stack.h>


/* external prototypes */
extern void int_vectors(void);


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

struct thread_ctx_t *avr_int_hdlr(struct thread_ctx_t *tc){
	uint8_t num;


	/* save thread context of active thread*/
	stack_push(sched_running()->ctx_stack, tc);

	/* compute interrupt number */
	// INT_VEC_SIZE / 2 is used since the flash is word-addressed
	num = (lo8(tc->int_vec_addr) << 8 | hi8(tc->int_vec_addr));
	num = ((num - (unsigned int)int_vectors) / (INT_VEC_SIZE / 2)) - 1;

	/* call interrupt handler */
	int_call(num);

	/* return context of active thread */
	BUILD_ASSERT(sizeof(errno_t) == 1);	// check sizeof thread_ctx_t::errno

	return stack_pop(((thread_t*)sched_running())->ctx_stack);
}

void avr_int_warm_reset_hdlr(void){
	kpanic("call reset handler without actual MCU reset\n");
}
