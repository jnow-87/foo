/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>


/* external functions */
.extern avr_init_bss
.extern avr_init_data
.extern _start


/* global functions */
.section .avr_start

.global avr_start
avr_start:
	/* ABI requirements */
	clr		r1		// r1 as zero register

	/* skip init if calling a thread, i.e. r24 and r25 are 0 */
	mov		r0, r24
	or		r0, r25
	brne	2f

	/**
	 * init .bss and .data
	 * 	ensuring _start() arg1 is not altered
	 */
	push	r22
	push	r23

	call	avr_init_bss
	call	avr_init_data

	pop		r23
	pop		r22

	/* set _start() arg0 */
	// compute _start() arg0
	// 	arg0 (r25|r24) needs to be set to _start(), due to word addressing
	// 	of the program memory r25|r24 has to be divided by 2
	clc
	ldi		r25, hh8(_start)
	cpi		r25, 0
	breq	1f
	sec

1:
	ldi		r25, hi8(_start)
	ror		r25
	ldi		r24, lo8(_start)
	ror		r24

2:
	/* call start */
	call	_start
