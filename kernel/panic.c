#include <arch/interrupt.h>
#include <arch/core.h>
#include <kernel/kprintf.h>


/* global functions */
void kernel_panic(void){
	int_enable(INT_NONE);
	FATAL("woops! halting core\n");
	core_halt();
}
