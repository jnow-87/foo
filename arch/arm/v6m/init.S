/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



.syntax unified


/* external variables */
.extern __data_start
.extern __data_end
.extern __data_load_start
.extern __bss_start
.extern __bss_end


/* global functions */
.global av6m_init_data
.type av6m_init_data,%function
av6m_init_data:
	ldr		r0, =__data_load_start
	ldr		r1, =__data_start
	ldr		r2, =__data_end
	b		2f

1:
	ldm		r0!, {r3}
	stm		r1!, {r3}

2:
	cmp		r1, r2
	blt		1b

	bx		lr

.global av6m_init_bss
.type av6m_init_bss,%function
av6m_init_bss:
	movs	r0, #0
	ldr		r1, =__bss_start
	ldr		r2, =__bss_end
	b		2f

1:
	stm		r1!, {r0}

2:
	cmp		r1, r2
	blt		1b

	bx		lr
