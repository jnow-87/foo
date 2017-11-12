#include <config/config.h>
#include <arch/arch.h>
#include <arch/interrupt.h>
#include <arch/core.h>
#include <arch/io.h>
#include <arch/avr/sched.h>
#include <kernel/init.h>
#include <kernel/kprintf.h>
#include <kernel/sched.h>
#include <kernel/panic.h>
#include <sys/errno.h>
#include <sys/register.h>


/* external prototypes */
extern void int_vectors(void);


/* global variables */
uint8_t int_num = 0;
uint8_t inkernel_nest = 0;


/* global functions */
struct thread_context_t *avr_int_hdlr(struct thread_context_t *tc){
	int terrno;


	/* save thread context if not interrupting the kernel */
	if(inkernel_nest == 1)
		sched_ctx_enqueue(tc);

	/* call respective interrupt handler */
	// save errno
	terrno = errno;
	errno = E_OK;


	// call handler
	switch(int_num){
	case INT_VECTORS:
		avr_sc_hdlr();
		break;

	case CONFIG_SCHED_INT:
		if(inkernel_nest == 1)
			avr_sched_hdlr();
		break;

	default:
		kpanic(sched_running(), "out of bound interrupt num %u\n", (unsigned int)int_num);
	}

	// restore errno
	errno = terrno;

	/* return context of active thread */
	return sched_ctx_dequeue();
}

void avr_int_warm_reset_hdlr(void){
	kpanic(0x0, "execute reset handler without actual MCU reset");
}

void avr_int_inval_hdlr(void){
	kpanic(0x0, "unhandled interrupt");
}

int avr_int_enable(int_type_t mask){
	if(mask)	asm volatile("sei");
	else		asm volatile("cli");

	return E_OK;
}

int_type_t avr_int_enabled(void){
	if(mreg_r(SREG) & (0x1 << SREG_I))
		return INT_GLOBAL;
	return INT_NONE;
}
