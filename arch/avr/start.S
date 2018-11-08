/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>


.extern kernel
.globl __start

__start:
	/* init stack pointer */
	ldi		r16, lo8(CONFIG_KERNEL_STACK_BASE + CONFIG_KERNEL_STACK_SIZE - 1)
	ldi		r17, hi8(CONFIG_KERNEL_STACK_BASE + CONFIG_KERNEL_STACK_SIZE - 1)
	sts		SPL, r16
	sts		SPH, r17

	/* load .data section */
/*{{{*/
	//	Z = __data_load_start
	ldi		r30, lo8(__data_load_start)
	ldi		r31, hi8(__data_load_start)

	// RAMPZ = __data_load_start[16..23]
#ifdef CONFIG_AVR_ISA_AVR51
	ldi		r16, hh8(__data_load_start)
	sts		RAMPZ, r16
#endif

	//	Y = __data_start
	ldi		r28, lo8(__data_start)
	ldi		r29, hi8(__data_start)

	// r16 = __data_end[8..15]
	ldi		r16, hi8(__data_end)

	rjmp	2f

1:
	// load from flash
#ifdef CONFIG_AVR_ISA_AVR51
	elpm	r1, Z+
#else
	lpm		r1, Z+
#endif

	// store to ram
	st		Y+, r1

2:
	// check done (Y == __data_end)
	cpi		r28, lo8(__data_end)
	cpc		r29, r16

	brne	1b

#ifdef CONFIG_AVR_ISA_AVR51
	clr		r16
	sts		RAMPZ, r16
#endif
/*}}}*/

	/* init .bss section */
	/*{{{*/
	ldi		r30, lo8(__bss_start)
	ldi		r31, hi8(__bss_start)
	ldi		r16, hi8(__bss_end)

	clr		r1

	rjmp	2f

1:
	st		Z+, r1

2:
	cpi		r30, lo8(__bss_end)
	cpc		r31, r16

	brne	1b
/*}}}*/

	/* goto kernel */
#ifdef CONFIG_ATMEGA88PA
	ldi		r30, lo8(init)
	ldi		r31, hi8(init)
	icall
#else
	call	kernel
#endif

	/* should never happen */
	sleep
