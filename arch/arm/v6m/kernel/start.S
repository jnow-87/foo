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
.extern kernel


/* global functions */
.global __av6m_start
.type __av6m_start,%function
__av6m_start:
	/* init .data and .bss */
	ldr		r0, =av6m_init_data
	blx		r0
	ldr		r0, =av6m_init_bss
	blx		r0

.global __av6m_start_multi_core
.type __av6m_start_multi_core,%function
__av6m_start_multi_core:
	/* disable interrupts */
	// on reset PRIMASK[PM] is set to 0, i.e. interrupts are enabled
	// to avoid issues during initialisation, disable them
	cpsid	i

	/* call kernel */
	ldr		r0, =kernel
	blx		r0

	/* should never happen */
1:
	wfe
	b		1b
