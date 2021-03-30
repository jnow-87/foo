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

	/* skip init if not calling the main thread */
	mov		r0, r24
	or		r0, r25
	brne	1f

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

1:
	/* call start */
	call	_start
