/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



.text

.global _start
_start:
	/* init */
	// mark end of stack
	xor		%rbp, %rbp

	/* call kernel */
	// ABI requirement
	xor		%rax, %rax
	call	kernel

	/* exit */
	// trigger linux exit syscall
	mov		%rax, %rdi
	mov		$60, %rax
	syscall
