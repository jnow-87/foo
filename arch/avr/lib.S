/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>


/* global functions */
.global avr_lib_crt0
avr_lib_crt0:
	push	r30
	push	r31
	push	r16
	push	r28
	push	r29
	push	r1

	/* load .data section */
/*{{{*/
	//	Z = __data_load_start
	ldi		r30, lo8(__data_load_start)
	ldi		r31, hi8(__data_load_start)

	// RAMPZ = __data_load_start[16..23]
#ifdef CONFIG_AVR_RAMPZ
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

#ifdef CONFIG_AVR_RAMPZ
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

	/* return 0 */
	pop		r1
	pop		r29
	pop		r28
	pop		r16
	pop		r31
	pop		r30

	ldi		r24, 0
	ldi		r25, 0

	ret
