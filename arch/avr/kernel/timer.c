/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <arch/avr/register.h>
#include <kernel/init.h>
#include <sys/devtree.h>


/* local functions */
static int init(void){
	avr_platform_cfg_t const *plt;


	plt = (avr_platform_cfg_t*)devtree_arch_payload("avr,platform");

	/* clear reset flags */
	mreg_w(MCUSR, 0x0);

	/* set watchdog prescale value and interrupt */
	asm volatile(
		"sts	%[reg], %[en]\n"
		"sts	%[reg], %[val]\n"
		:
		: [reg] "i" (WDTCSR),
		  [en] "r" ((0x1 << WDTCSR_WDCE) | (0x1 << WDTCSR_WDE)),
		  [val] "r" ((0x1 << WDTCSR_WDIE) | (plt->watchdog_prescaler << WDTCSR_WDP0))
	);

	return 0;
}

platform_init(1, first, init);
