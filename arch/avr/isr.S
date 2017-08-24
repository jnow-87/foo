#include <config/config.h>
#include <arch/arch.h>


/* macros */
#define ISR(name, tgt) \
.global __isr_##name $ \
__isr_##name: \
	ICALL tgt


/* external functions */
.extern __start


/* external variables */
.extern inkernel_nest
.extern kernel_stack


/* interrupt vectors */
.section .reset

ISR(reset, __start)
ISR(int0, __isr_call)
ISR(int1, __isr_call)
ISR(int2, __isr_call)
ISR(pcint0, __isr_call)
ISR(pcint1, __isr_call)
ISR(pcint2, __isr_call)
ISR(pcint3, __isr_call)
ISR(watchdog, __isr_call)
ISR(timer2_comp_a, __isr_call)
ISR(timer2_comp_b, __isr_call)
ISR(timer2_ovfl, __isr_call)
ISR(timer1_capture, __isr_call)
ISR(timer1_comp_a, __isr_call)
ISR(timer1_comp_b, __isr_call)
ISR(timer1_ovfl, __isr_call)
ISR(timer0_comp_a, __isr_call)
ISR(timer0_comp_b, __isr_call)
ISR(timer0_ovfl, __isr_call)
ISR(spi_tx, __isr_call)
ISR(usart0_rx, __isr_call)
ISR(usart0_udre, __isr_call)
ISR(usart0_tx, __isr_call)
ISR(analog_comp, __isr_call)
ISR(adc, __isr_call)
ISR(eeprom_rdy, __isr_call)
ISR(twi, __isr_call)
ISR(spm_rdy, __isr_call)
ISR(usart1_rx, __isr_call)
ISR(usart1_udre, __isr_call)
ISR(usart1_tx, __isr_call)
ISR(timer3_capture, __isr_call)
ISR(timer3_comp_a, __isr_call)
ISR(timer3_comp_b, __isr_call)
ISR(timer3_ovfl, __isr_call)


.section .text

/* ISR */
.global __isr_call
__isr_call:
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

	// save last register
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

	/* pop interrupt vector address */
	pop		r0
	pop		r0

	reti
