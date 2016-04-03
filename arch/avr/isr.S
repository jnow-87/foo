#include <arch/arch.h>


/* macros */
#define isr(name, tgt) \
.global __isr_##name $ \
__isr_##name: \
	ICALL tgt


/* external variables */
.extern inkernel_nest


/* interrupt vectors */
.section .reset

isr(reset, __start)
isr(int0, __isr_call)
isr(int1, __isr_call)
isr(int2, __isr_call)
isr(pcint0, __isr_call)
isr(pcint1, __isr_call)
isr(pcint2, __isr_call)
isr(pcint3, __isr_call)
isr(watchdog, __isr_call)
isr(timer2_comp_a, __isr_call)
isr(timer2_comp_b, __isr_call)
isr(timer2_ovfl, __isr_call)
isr(timer1_capture, __isr_call)
isr(timer1_comp_a, __isr_call)
isr(timer1_comp_b, __isr_call)
isr(timer1_ovfl, __isr_call)
isr(timer0_comp_a, __isr_call)
isr(timer0_comp_b, __isr_call)
isr(timer0_ovfl, __isr_call)
isr(spi_tx, __isr_call)
isr(usart0_rx, __isr_call)
isr(usart0_udre, __isr_call)
isr(usart0_tx, __isr_call)
isr(analog_comp, __isr_call)
isr(adc, __isr_call)
isr(eeprom_rdy, __isr_call)
isr(twi, __isr_call)
isr(spm_rdy, __isr_call)
isr(usart1_rx, __isr_call)
isr(usart1_udre, __isr_call)
isr(usart1_tx, __isr_call)
isr(timer3_capture, __isr_call)
isr(timer3_comp_a, __isr_call)
isr(timer3_comp_b, __isr_call)
isr(timer3_ovfl, __isr_call)


.section .text

/* ISR */
.global __isr_call
__isr_call:
	/* save ISR return address
	 *	store return address to GPIOR1, GPRIO2
	 */
	sts		GPIOR0, r16
	pop		r16
	sts		GPIOR1, r16
	pop		r16
	sts		GPIOR2, r16
	lds		r16, GPIOR0

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

	/* avr_int_hdlr arg1: stack pointer aka thread_context_t pointer */
	lds		r22, SPL
	lds		r23, SPH

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

	// init kernel stack pointer
	ldi		r17, lo8(KERNEL_STACK_BASE + KERNEL_STACK_SIZE - 1)
	ldi		r18, hi8(KERNEL_STACK_BASE + KERNEL_STACK_SIZE - 1)
	sts		SPL, r17
	sts		SPH, r18

	/* ABI requirements */
	clr		r1

1:
	/* increment inkernel_nest */
	inc		r16
	sts		inkernel_nest, r16

	/* avr_int_hdlr arg0: ISR return address */
	lds		r25, GPIOR1
	lds		r24, GPIOR2

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

	reti
