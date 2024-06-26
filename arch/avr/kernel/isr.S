/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <sys/devicetree.h>


/* macros */
.macro	int_lvl0 num
	.if (\num < INT_NUM_HWINTS)
		.if(\num == 0)					// reset
			XJMP	__isr_reset
		.else
			XJMP	__int_lvl1_\num
		.endif
	.endif
.endm

.macro int_lvl1 num
	.if (\num < INT_NUM_HWINTS)
		__int_lvl1_\num:
			XCALL	__isr_int
			ret
			nop
	.endif
.endm


/* external functions */
.extern __start
.extern avr_int_hdlr
.extern avr_int_warm_reset_hdlr


/* data */
.section .bss

zero: .byte 1


/* interrupt vectors */
.section .reset

.global int_vectors_lvl0
int_vectors_lvl0:
int_lvl0 0
int_lvl0 1
int_lvl0 2
int_lvl0 3
int_lvl0 4
int_lvl0 5
int_lvl0 6
int_lvl0 7
int_lvl0 8
int_lvl0 9
int_lvl0 10
int_lvl0 11
int_lvl0 12
int_lvl0 13
int_lvl0 14
int_lvl0 15
int_lvl0 16
int_lvl0 17
int_lvl0 18
int_lvl0 19
int_lvl0 20
int_lvl0 21
int_lvl0 22
int_lvl0 23
int_lvl0 24
int_lvl0 25
int_lvl0 26
int_lvl0 27
int_lvl0 28
int_lvl0 29
int_lvl0 30
int_lvl0 31
int_lvl0 32
int_lvl0 33
int_lvl0 34
int_lvl0 35
int_lvl0 36

.global int_vectors_lvl1
int_vectors_lvl1:
int_lvl1 0
int_lvl1 1
int_lvl1 2
int_lvl1 3
int_lvl1 4
int_lvl1 5
int_lvl1 6
int_lvl1 7
int_lvl1 8
int_lvl1 9
int_lvl1 10
int_lvl1 11
int_lvl1 12
int_lvl1 13
int_lvl1 14
int_lvl1 15
int_lvl1 16
int_lvl1 17
int_lvl1 18
int_lvl1 19
int_lvl1 20
int_lvl1 21
int_lvl1 22
int_lvl1 23
int_lvl1 24
int_lvl1 25
int_lvl1 26
int_lvl1 27
int_lvl1 28
int_lvl1 29
int_lvl1 30
int_lvl1 31
int_lvl1 32
int_lvl1 33
int_lvl1 34
int_lvl1 35
int_lvl1 36


/* isrs */
.section .isr

// reset handler
.global __isr_reset
__isr_reset:
	/* ABI requirements */
	clr		r1	// r1 as zero register

	/* check if an interrupt source is present if not,
	 * the reset vector is triggered due to an error
	 */
	lds		r16, MCUSR
	cpi		r16,0x0
	brne	__start
	XJMP	avr_int_warm_reset_hdlr

// generic isr
.global __isr_int
__isr_int:
	/* save registers */
	push	r31
	push	r30
	push	r29
	push	r28
	push	r27
	push	r26
	push	r25
	push	r24
	push	r23
	push	r22
	push	r21
	push	r20
	push	r19
	push	r18
	push	r17
	push	r16
	push	r15
	push	r14
	push	r13
	push	r12
	push	r11
	push	r10
	push	r9
	push	r8
	push	r7
	push	r6
	push	r5
	push	r4
	push	r3
	push	r2
	push	r1
	push	r0

	lds		r16, GPIOR2
	push	r16
	lds		r16, GPIOR1
	push	r16
	lds		r16, GPIOR0
	push	r16

#ifdef CONFIG_AVR_RAMPZ
	lds		r16, RAMPZ
#else
	ldi		r16, 0
#endif
	push	r16
	lds		r16, MCUSR
	push	r16
	lds		r16, SREG
	push	r16

	/* reserve space for thread_ctx_t control data
	 * and prepare interrupt hander arguments
	 */
	// push dummies to reserve space for
	//	thread_ctx_t::next
	//	thread_ctx_t::this
	//	thread_ctx_t::type
	//
	//	the last dummy is pushed once the thread_ctx_t pointer has
	//	been saved, this is required due to the post-decrement of
	//	the push instruction
	push	r1
	push	r1

#if (DEVTREE_ARCH_ADDR_WIDTH == 16)
	push	r1
	push	r1
#endif

	// avr int hdlr arg: stack pointer aka thread_ctx_t pointer
	lds		r24, SPL
	lds		r25, SPH

	// push last thread_ctx_t dummy byte
	push	r1

	/* ABI requirements */
	clr		r1	// r1 as zero register

	XCALL	avr_int_hdlr

	// restore thread stack pointer from return value
	sbiw	r24, 0x1	// decrement due to pre-increment of pop
	sts		SPL, r24
	sts		SPH, r25

	/* pop thread_ctx_t dummy bytes */
#if (DEVTREE_ARCH_ADDR_WIDTH == 16)
	pop		r16
	pop		r16
#endif

	pop		r16
	pop		r16
	pop		r16

	/* restore regsiters */
	pop		r16
	sts		SREG, r16
	pop		r16
	sts		MCUSR, r16
	pop		r16
#ifdef CONFIG_AVR_RAMPZ
	sts		RAMPZ, r16
#endif

	pop		r16
	sts		GPIOR0, r16
	pop		r16
	sts		GPIOR1, r16
	pop		r16
	sts		GPIOR2, r16

	pop		r0
	pop		r1
	pop		r2
	pop		r3
	pop		r4
	pop		r5
	pop		r6
	pop		r7
	pop		r8
	pop		r9
	pop		r10
	pop		r11
	pop		r12
	pop		r13
	pop		r14
	pop		r15
	pop		r16
	pop		r17
	pop		r18
	pop		r19
	pop		r20
	pop		r21
	pop		r22
	pop		r23
	pop		r24
	pop		r25
	pop		r26
	pop		r27
	pop		r28
	pop		r29
	pop		r30
	pop		r31

	reti
