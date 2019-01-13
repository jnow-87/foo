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
	.if (\num < INT_VECTORS + 2)				// +2 for the pseude interrupts (syscall, instruction overflow)
		.if (\num == 0)							// reset
			.global __vector_\num
			XJMP	__isr_reset

		.elseif (\num == INT_VECTORS)			// syscall
			.global	__vector_\num
			XJMP	__isr_syscall

#if (defined(CONFIG_BUILD_DEBUG) && defined(CONFIG_IOVERFLOW_DET))
		.elseif (\num == INT_VECTORS + 1)		// instruction overflow
			.global	__vector_\num
			XJMP	__isr_iovfl
#endif // CONFIG_BUILD_DEBUG && CONFIG_IOVERFLOW_DET

#if (defined(CONFIG_SCHED_PREEMPTIVE) || defined(CONFIG_KERNEL_TIMER))
		.elseif (\num == CONFIG_TIMER_INT)		// scheduler
			.global	__vector_\num
			XJMP	__isr_sched
#endif // CONFIG_SCHED_PREEMPTIVE || CONFIG_KERNEL_TIMER

		.else									// hardware interrupt
			.global	__vector_\num
			.weak	__vector_\num
			.set	__vector_\num , __isr_inval
			XJMP	__vector_\num
		.endif
	.endif
.endm


/* external functions */
.extern __start
.extern avr_int_hdlr
.extern avr_int_warm_reset_hdlr
.extern avr_int_inval_hdlr


/* external variables */
.extern int_num


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


.section .text
/* reset handler */
.global __isr_reset
__isr_reset:
	/* check if an interrupt source is present if not,
	 * the reset vector is triggered due to an error
	 */
	lds		r16, MCUSR
	cpi		r16,0x0
	brne	__start
	XJMP	avr_int_warm_reset_hdlr

/* syscall handler */
.global __isr_syscall
__isr_syscall:
	push	r31
	ldi		r31, INT_VECTORS
	sts		int_num, r31
	XJMP	__int_hdlr_call

/* scheduler/timer handler */
#if (defined(CONFIG_SCHED_PREEMPTIVE) || defined(CONFIG_KERNEL_TIMER))
.global __isr_sched
__isr_sched:
	push	r31
	ldi		r31, CONFIG_TIMER_INT
	sts		int_num, r31
	XJMP	__int_hdlr_call
#endif // CONFIG_SCHED_PREEMPTIVE || CONFIG_KERNEL_TIMER

/* instruction overflow handler */
#if (defined(CONFIG_BUILD_DEBUG) && defined(CONFIG_IOVERFLOW_DET))
__isr_iovfl:
	push	r31
	ldi		r31, INT_VECTORS + 1
	sts		int_num, r31
	XJMP	__int_hdlr_call
#endif // CONFIG_BUILD_DEBUG && CONFIG_IOVERFLOW_DET

/* invalid interrupt handler */
// NOTE using avr_int_inval_hdlr directly does not work, since
//		somehow the .weak symbols are not redefined in this case
.global __isr_inval
__isr_inval:
	XJMP	avr_int_inval_hdlr


/* ISR with context handling */
.global __int_hdlr_call
__int_hdlr_call:
	/* save registers */
	// NOTE r31 is assumed being pushed by the calling routine
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

	lds		r16, RAMPZ
	push	r16
	lds		r16, MCUSR
	push	r16
	lds		r16, SREG
	push	r16

	/* reserve space for thread_ctx_t::next and prepare interrupt hander arguments */
	// push dummy to reserve space for next member of thread_ctx_t
#if (CONFIG_ADDR_WIDTH == 16)
	push	r1
#endif

	// avr_int_hdlr argument: stack pointer aka thread_ctx_t pointer
	lds		r24, SPL
	lds		r25, SPH

	// push last dummy byte for next, prev members of thread_ctx_t
	// thread_context pointer needs to be saved earlier due to post-decrement of push
	push	r1

	/* ABI requirements */
	// clear r1
	clr		r1

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
	sts		SREG, r16
	pop		r16
	sts		MCUSR, r16
	pop		r16
	sts		RAMPZ, r16

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
