/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



.syntax unified


/* external functions */
.extern av6m_init_data
.extern av6m_init_bss
.extern _start


/* global functions */
.section .arm_start

.global arm_start
arm_start:
	/* skip init if not calling the main thread */
	cmp		r0, 0
	bne		1f

	/* init .data and .bss */
	ldr		r2, =av6m_init_data
	blx		r2
	ldr		r2, =av6m_init_bss
	blx		r2

1:
	ldr		r2, =_start
	bx		r2
