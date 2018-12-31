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
#include <kernel/init.h>
#include <kernel/kprintf.h>
#include <kernel/sched.h>
#include <kernel/panic.h>
#include <sys/errno.h>
#include <sys/stack.h>
#include <sys/register.h>


/* external prototypes */
extern void int_vectors(void);


/* global variables */
uint8_t int_num = 0;


/* global functions */
struct thread_context_t *avr_int_hdlr(struct thread_context_t *tc){
	errno_t terrno;


	/* save thread context of active thread*/
	stack_push(sched_running()->ctx_stack, tc);

	/* call respective interrupt handler */
	// save and reset errno
	terrno = errno;
	errno = E_OK;

	// call handler
	switch(int_num){
	case INT_VECTORS:		// syscall
		avr_sc_hdlr();
		break;

#if (defined(CONFIG_BUILD_DEBUG) && defined(CONFIG_IOVERFLOW_DET))
	case INT_VECTORS + 1:	// instruction overflow
		avr_iovfl_hdlr(tc);
		break;
#endif // CONFIG_BUILD_DEBUG && CONFIG_IOVERFLOW_DET

#if (defined(CONFIG_SCHED_PREEMPTIVE) || defined(CONFIG_KERNEL_TIMER))
	case CONFIG_TIMER_INT:	// scheduler
		avr_timer_hdlr();
		break;
#endif // CONFIG_SCHED_PREEMPTIVE || CONFIG_KERNEL_TIMER

	default:
		kpanic(sched_running(), "out of bound interrupt num %u\n", (unsigned int)int_num);
	}

	// restore errno
	errno = terrno;

	/* return context of active thread */
	return stack_pop(((thread_t*)sched_running())->ctx_stack);
}

void avr_int_warm_reset_hdlr(void){
	kpanic(0x0, "execute reset handler without actual MCU reset");
}

void avr_int_inval_hdlr(void){
	kpanic(0x0, "unhandled interrupt");
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
