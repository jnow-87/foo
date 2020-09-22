/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



.data

/* global variables */
.global argc
argc:	.zero	4

.global argv
argv:	.zero	8

.global envp
envp:	.zero	8


.text

/* global functions */
.global x86_start
x86_start:
	/* get argc, argv, envp */
	mov		(%rsp), %rax
	mov		%rax, argc
	lea		8(%rsp), %rax
	mov		%rax, argv
	lea		16(%rsp, %rdi, 8), %eax
	mov		%eax, envp

	/* ABI requirement */
	xor		%rax, %rax

	/* _start */
	// prepare args
	mov		$_start, %rdi
	mov		8(%rsp), %rsi

	call	_start
