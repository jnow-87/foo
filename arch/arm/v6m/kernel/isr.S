/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/memory.h>
#include <sys/devicetree.h>


.syntax unified


/* external functions */
.extern __av6m_start
.extern av6m_int_hdlr
.extern av6m_inval_hdlr


/* vector table */
.section .vector.table

.global vector_table
vector_table:
	// stack pointer
.word	KERNEL_STACK(0)

	// arm v6-m exceptions
.word	__av6m_start			// reset
.word	__isr_int				// nmi
.word	__isr_int				// hard fault
.word	av6m_inval_hdlr			// reserved
.word	av6m_inval_hdlr			// reserved
.word	av6m_inval_hdlr			// reserved
.word	av6m_inval_hdlr			// reserved
.word	av6m_inval_hdlr			// reserved
.word	av6m_inval_hdlr			// reserved
.word	av6m_inval_hdlr			// reserved
.word	__isr_int				// svcall
.word	av6m_inval_hdlr			// reserved
.word	av6m_inval_hdlr			// reserved
.word	__isr_int				// pendsv
.word	__isr_int				// systick

	// external interrupts handled by nvic
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int
.word	__isr_int


/* local functions */
.type __isr_int,%function
__isr_int:
	/* save registers */
	// push lo and hi registers
	mov		r0, r8
	mov		r1, r9
	mov		r2, r10
	mov		r3, r11
	push	{r0-r7,lr}
	mrs		r0, CONTROL
	push	{r0}

	/* call call handler */
	// reserve space for  thread_ctx_t control data
	//	next
	//	this
	//	type
	sub		sp, 12

	// prepare arg0 (thread context)
	mov		r0, sp

	cpsid	i
	ldr		r4, =av6m_int_hdlr
	blx		r4
	cpsie	i

	// restore stack pointer, removing thread_ctx_t control data
	mov		sp, r0
	add		sp, 12

	/* restore registers */
	// pop lo and hi registers
	pop		{r0}
	msr		CONTROL, r0
	isb
	pop		{r0-r7}
	mov		r11, r3
	mov		r10, r2
	mov		r9, r1
	mov		r8, r0

	pop		{pc}
