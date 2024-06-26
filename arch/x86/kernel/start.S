/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



/* external functions */
.extern x86_std_init
.extern x86_std_fini
.extern x86_opts_parse
.extern kernel


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
.global _start
_start:
	/* init */
	// mark end of stack
	xor		%rbp, %rbp

	/* get argc, argv, envp */
	mov		(%rsp), %rax
	mov		%rax, argc
	lea		8(%rsp), %rax
	mov		%rax, argv
	lea		16(%rsp, %rdi, 8), %eax
	mov		%eax, envp

	/* ABI requirement */
	xor		%rax, %rax

	/* init */
	call	x86_std_init
	call	x86_opts_parse

	/* kernel */
	call	kernel

	/* fini */
	call	x86_std_fini

	/* exit */
	// trigger linux exit syscall
	mov		%rax, %rdi
	mov		$60, %rax
	syscall
