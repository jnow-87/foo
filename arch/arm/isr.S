/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>


.extern __start

.section .reset

int_vector_table:
	.word	CONFIG_KERNEL_STACK_BASE
	.word	__start
	.word	__start
	.word	__start
	.word	__start
	.word	__start
	.word	__start
	.word	__start
	.word	__start
	.word	__start
	.word	__start
	.word	__start
	.word	__start
	.word	__start
	.word	__start
