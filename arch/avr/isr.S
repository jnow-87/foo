#include <arch/arch.h>


/* macros */
#define isr(name, tgt) \
.global __isr_##name $ \
__isr_##name: \
	rcall tgt


/* external symbols */
.extern isr_hdlr


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

/* save context */
__ctx_save:	.macro
			.endm

/* restore context */
__ctx_restore:	.macro
				.endm


/* ISR implementation */
.global __isr_call
__isr_call:
	/* save context */
	__ctx_save

	/* load stack pointer to Z */
	lds		r30, SPL
	lds		r31, SPH

	/* load return-address from stack to Y */
	adiw	r30, 1
	ld		r29, Z
	adiw	r30, 1
	ld		r28, Z

	// translate return-address to byte address
	lsl		r28
	rol		r29

	/** compute offset into isr_hdlr[], using the offset of the return address into the
	 *  interrupt vector table
	 *
	 *  the return address is one element ahead from the desired offset
	 *
	 *  offset =
	 *    (((ret_addr - int_vec_base - size of int_vec_base) / (size of int_vec_entry))) * sizeof(isr_hdlr)
	 *
	 *    size of int_vec_base = 2 (cf. AVR ISA size of rcall instruction)
	 *    sizeof(isr_hdlr) = 2
	 *
	 *    => no adaption of the index required, since both array element types (int_vec and isr_hdlr)
	 *       have the same size
	 */

	// load interrupt vector base -1 element
	ldi		r30, lo8(__isr_reset + 2)
	ldi		r31, hi8(__isr_reset + 2)

	// ret_addr - __isr_base
	sub		r28, r30
	sbc		r29, r31

	// load isr_hdlr to Z
	ldi		r30, lo8(isr_hdlr)
	ldi		r31, hi8(isr_hdlr)

	// apply computed offset to isr_hdlr
	add		r28, r30
	adc		r29, r31

	// check if handler is 0x0
	ld		r30, Y
	ldd		r31, Y+1

	ldi		r28, 0x0

	cpi		r30, 0x0
	cpc		r31, r28

	breq	__isr_zero_hdlr

	/* call the handler from Z */
	icall

	/* clear stack arguments pushed by the interrupt vector 'call' instruction*/
	pop		r1
	pop		r1

	/* restore context */
	__ctx_restore

	reti

/* unhandled interrupt */
__isr_zero_hdlr:
	sleep
