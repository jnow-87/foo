/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



/* global functions */
.global x86_start
x86_start:
	mov		$_start, %rdi
	mov		8(%rsp), %rsi
	call	_start

.global x86_lib_crt0
x86_lib_crt0:
	mov		$0x0, %eax
	retq
