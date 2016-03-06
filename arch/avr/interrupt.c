#include <arch/arch.h>
#include <arch/interrupt.h>
#include <kernel/kprintf.h>
#include <sys/error.h>


/* global variables */
// XXX: used in assembly ISR handler
int_hdlr_t isr_hdlr[NINTERRUPTS] = { 0x0 };


/* global functions */
error_t avr_int_enable(int_type_t mask){
	if(mask)	asm volatile("sei");
	else		asm volatile("cli");

	return E_OK;
}

int_type_t avr_int_enabled(void){
	if(mreg_r(SREG) & (0x1 << SREG_I))
		return INT_GLOBAL;
	return INT_NONE;
}

error_t avr_int_hdlr_register(int_num_t num, int_hdlr_t hdlr){
	if(isr_hdlr[num] != 0x0){
		WARN("interrupt already registerd %u %#x\n", num, isr_hdlr[num]);
		return E_INUSE;
	}

	isr_hdlr[num] = hdlr;

	return E_OK;
}

error_t avr_int_hdlr_release(int_num_t num){
	isr_hdlr[num] = 0x0;
	return E_OK;
}
