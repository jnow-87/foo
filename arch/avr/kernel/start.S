/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <sys/devicetree.h>


/* external functions */
.extern avr_init_bss
.extern avr_init_data
.extern kernel


/* global functions */
.section .start

.global __start
__start:
	/* init stack pointer */
	ldi		r16, lo8(DEVTREE_KERNEL_STACK_BASE + DEVTREE_KERNEL_STACK_SIZE - 1)
	ldi		r17, hi8(DEVTREE_KERNEL_STACK_BASE + DEVTREE_KERNEL_STACK_SIZE - 1)
	sts		SPL, r16
	sts		SPH, r17

	/* init .bss and .data */
	call	avr_init_bss
	call	avr_init_data

	/* ABI requirements */
	clr		r1	// r1 as zero register

	/* kernel */
#ifdef CONFIG_AVR_ISA_AVR4
	ldi		r30, lo8(kernel)
	ldi		r31, hi8(kernel)
	icall
#else
	call	kernel
#endif

	/* should never happen */
	sleep
