/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>


/* macros */
.macro	vector num
	.if (\num <= INT_VECTORS)
		.if (\num == INT_RESET)
			.global __vector_\num
			.word	__start

		.else
			.global	__vector_\num
			.weak	__vector_\num
			.set	__vector_\num , __isr_inval
			.word	__vector_\num
		.endif
	.endif
.endm


/* external functions */
.extern __start


/* interrupt vectors */
.section .reset

.global int_vectors
int_vectors:
	// initial stack pointer
	.word	CONFIG_KERNEL_STACK_BASE + CONFIG_KERNEL_STACK_SIZE - 4

	// system exceptions
	vector 1
	vector 2
	vector 3
	vector 4
	vector 5
	vector 6
	vector 7
	vector 8
	vector 9
	vector 10
	vector 11
	vector 12
	vector 13
	vector 14
	vector 15
	vector 16

	// external exceptions
	// TODO


/* ISRs */
.section .text
/* invalid interrupt handler */
.global __isr_inval
.type __isr_inval, %function
__isr_inval:
	bkpt
