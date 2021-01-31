/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>


/* global functions */
long int lnx_syscall(unsigned long int num, unsigned long int args[6], int tolerant){
	register long int r;


	while(1){
		asm volatile(
			"movq	0(%[args]), %%rdi\n"
			"movq	8(%[args]), %%rsi\n"
			"movq	16(%[args]), %%rdx\n"
			"movq	24(%[args]), %%r10\n"
			"movq	32(%[args]), %%r8\n"
			"movq	40(%[args]), %%r9\n"
			"movq	%[num], %%rax\n"
			"syscall\n"
			"movq	%%rax, %[r]\n"
			: [r] "=a" (r)
			: [num] "m" (num),
			  [args] "c" (args)
			: "memory"
		);

		if(!tolerant)
			return r;

		switch(r){
		case -4: // EINTR
			LNX_DEBUG("tolerated linux syscall error %d\n", r);
			break;

		default:
			return r;
		}
	}
}
