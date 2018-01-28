#include <config/config.h>
#include <arch/arch.h>
#include <arch/interrupt.h>
#include <arch/core.h>
#include <arch/io.h>
#include <arch/avr/timer.h>
#include <kernel/init.h>
#include <kernel/kprintf.h>
#include <kernel/thread.h>
#include <kernel/sched.h>
#include <kernel/panic.h>
#include <sys/errno.h>
#include <sys/register.h>


/* external prototypes */
extern void int_vectors(void);


/* global variables */
uint8_t int_num = 0;


/* global functions */
struct thread_context_t *avr_int_hdlr(struct thread_context_t *tc){
	int terrno;


	/* save thread context of active thread*/
	thread_context_enqueue((thread_t*)sched_running(), tc);

	/* call respective interrupt handler */
	// save errno
	terrno = errno;
	errno = E_OK;

	// call handler
	switch(int_num){
	case INT_VECTORS:
		avr_sc_hdlr();
		break;

	case CONFIG_TIMER_INT:
		avr_timer_hdlr();
		break;

	default:
		kpanic(sched_running(), "out of bound interrupt num %u\n", (unsigned int)int_num);
	}

	// restore errno
	errno = terrno;

	/* return context of active thread */
	return thread_context_dequeue((thread_t*)sched_running());
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
