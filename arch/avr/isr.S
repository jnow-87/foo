/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>


/* macros */
.macro	vector num
	.if (\num < NUM_INT)
		.if(\num == 0)					// reset
			.global __vector_\num
			XJMP	__isr_reset

		.else
			.global	__vector_\num
			XCALL	__isr_int
		.endif
	.endif
.endm


/* external variables */
.extern errno


/* external functions */
.extern __start
.extern avr_int_hdlr
.extern avr_int_warm_reset_hdlr


/* data */
.section .bss
zero: .byte 1


/* interrupt vectors */
.section .reset

.global int_vectors
int_vectors:
vector 0
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
vector 17
vector 18
vector 19
vector 20
vector 21
vector 22
vector 23
vector 24
vector 25
vector 26
vector 27
vector 28
vector 29
vector 30
vector 31
vector 32
vector 33
vector 34
vector 35
vector 36


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

	lds		r16, errno
	push	r16

	/* reserve space for thread_ctx_t::next and prepare interrupt hander arguments */
	// push dummy to reserve space for next member of thread_ctx_t
#if (CONFIG_ADDR_WIDTH == 16)
	push	r1
#endif

	// avr_sys_int_hdlr argument: stack pointer aka thread_ctx_t pointer
	lds		r24, SPL
	lds		r25, SPH

	// push last dummy byte for next, prev members of thread_ctx_t
	// thread_context pointer needs to be saved earlier due to post-decrement of push
	push	r1

	/* ABI requirements */
	clr		r1	// r1 as zero register

1:
	XCALL	avr_int_hdlr

	// restore thread stack pointer from return value
	sbiw	r24, 0x1	// decrement due to pre-increment of pop
	sts		SPL, r24
	sts		SPH, r25

1:
	/* pop dummy bytes used to reserve space for next member of thread_ctx_t */
#if (CONFIG_ADDR_WIDTH == 16)
	pop		r16
#endif

	pop		r16

	/* restore regsiters */
	pop		r16
	sts		errno, r16

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

	/* pop int_vectors return address */
	pop		r1
	pop		r1
	lds		r1, zero

	reti
