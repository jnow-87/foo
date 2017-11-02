#include <config/config.h>
#include <arch/arch.h>


/* macros */
#define		ISR_USE_CALL	0


.macro	vector num
	.if (\num < INT_VECTORS)
		.if (\num == CONFIG_SC_INT)
			.global	__vector_\num
#if ISR_USE_CALL == 0
			jmp	__isr_ctx
#else
			ICALL	__isr_ctx
#endif // ISR_USE_CALL

		.elseif (\num == CONFIG_SCHED_INT)
			.global	__vector_\num
#if ISR_USE_CALL == 0
			jmp	__isr_ctx_sched
#else
			ICALL	__isr_ctx_sched
#endif // ISR_USE_CALL

		.elseif (\num == 0)
			.global __vector_\num
			jmp		__isr_reset

		.else
			jmp		__isr_inval
		.endif
	.endif
.endm


/* external functions */
.extern __start
.extern avr_int_hdlr
.extern avr_int_warm_reset_hdlr
.extern avr_int_inval_hdlr


/* external variables */
.extern inkernel_nest
.extern kernel_stack


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


/* reset handler */
.section .text

.global __isr_ctx_sched
__isr_ctx_sched:
#if ISR_USE_CALL != 0
	// pop interrupt vector return address
	pop		r1
	pop		r1
	clr		r1
#endif // ISR_USE_CALL

	reti


.global __isr_reset
__isr_reset:
	/* check if an interrupt source is present if not,
	 * the reset vector is triggered due to an error
	 */
	lds		r16, MCUSR
	cpi		r16,0x0
	brne	__start
	jmp		avr_int_warm_reset_hdlr


/* invalid interrupt handler
 *	NOTE: using avr_int_inval_hdlr directly does not work, since
 *		  somehow the .weak symbols are not redefined in this case
 */
.global __isr_inval
__isr_inval:
	jmp	avr_int_inval_hdlr


/* ISR with context handling */
.global __isr_ctx
__isr_ctx:
	/* save context */
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

	lds		r16, RAMPZ
	push	r16
	lds		r16, MCUSR
	push	r16
	lds		r16, SREG

	/* avr_int_hdlr argument: stack pointer aka thread_context_t pointer */
	lds		r24, SPL
	lds		r25, SPH

	// push SREG
	// thread_context pointer needs to be saved earlier due to post-decrement of push
	push	r16

	/* update stack pointer
	 *	if already in kernel (inkernel_nest != 0) do nothing
	 *	else set stack pointer to initial kernel stack
	 */
	// check if already in kernel
	lds		r16, inkernel_nest
	cpi		r16, 0x0
	brne	1f

	// Z = kernel stack address
	ldi		r30, lo8(kernel_stack)
	ldi		r31, hi8(kernel_stack)

	// RAMPZ = kernel_stack[16..23]
#ifdef CONFIG_AVR_ISA_AVR51
	ldi		r16, hh8(kernel_stack)
	sts		RAMPZ, r16
#endif

#if CONFIG_NCORES > 1
	// assuming a single core, thus kernel_stack[] contains a single entry
	#error "handling of multiple cores is not implemented for AVR processors"
#endif // CONFIG_NCORES

	// load kernel stack from memory
	ld		r17, Z+
	ld		r18, Z
	sts		SPL, r17
	sts		SPH, r18

	/* ABI requirements */
	// clear r1
	clr		r1

1:
	/* increment inkernel_nest */
	lds		r16, inkernel_nest
	inc		r16
	sts		inkernel_nest, r16

	ICALL	avr_int_hdlr

	/* decrement inkernel_nest */
	lds		r16, inkernel_nest
	dec		r16
	sts		inkernel_nest, r16

	/* restore stack pointer
	 *	if leaving kernel (inkernel_nest == 0) restore stack pointer from thread context
	 *	else do nothing
	 */
	cpi		r16, 0x0
	brne	1f

	// restore thread stack pointer from return value
	sbiw	r24, 0x1	// decrement due to pre-increment of pop
	sts		SPL, r24
	sts		SPH, r25

1:
	/* restore context */
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

#if ISR_USE_CALL != 0
	// pop interrupt vector return address
	pop		r1
	pop		r1
	clr		r1
#endif // ISR_USE_CALL

	reti
