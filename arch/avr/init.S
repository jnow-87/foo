/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/avr/atmega.h>


/* external variables */
.extern __bss_start
.extern __bss_end
.extern __data_load_start
.extern __data_load_end
.extern __data_start
.extern __data_end


/* global functions */
// .bss init
//	NOTE function does not save register values
.global avr_init_bss
avr_init_bss:
	ldi		r30, lo8(__bss_start)
	ldi		r31, hi8(__bss_start)
	ldi		r16, hi8(__bss_end)

	rjmp	2f

1:
	st		Z+, r1

2:
	cpi		r30, lo8(__bss_end)
	cpc		r31, r16
	brne	1b

	ret

// .data init
//	NOTE function does not save register values
.global avr_init_data
avr_init_data:
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
	elpm	r0, Z+
#else
	lpm		r0, Z+
#endif

	// store to ram
	st		Y+, r0

2:
	// check done (Y == __data_end)
	cpi		r28, lo8(__data_end)
	cpc		r29, r16
	brne	1b

	ret
