#include <arch/arch.h>
#include <kernel/init.h>
#include <sys/error.h>


/* global functions */
void avr_core_sleep(void){
	asm volatile("sleep");
}

void avr_core_halt(void){
	asm volatile(
		"break\n"
		"sleep\n"
	);
}


/* local functions */
static error_t avr_core_init(void){
	/* set MCUCR[IVSEL]
	 * 	moving interrupt vectors to boot flash */
	mreg_bset_sync(MCUCR, MCUCR_IVSEL, MCUCR_IVCE);

	return E_OK;
}

core_init(0, avr_core_init);
