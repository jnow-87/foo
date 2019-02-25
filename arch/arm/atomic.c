/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <sys/types.h>


/* global functions */
int arm_cas(int volatile *v, int old, int new){
	uint32_t t;


	asm volatile(
		"ldrex		%[tmp], [%[v]]\n"
		"cmp		%[tmp], %[old]\n"
		"it			ne\n"
		"clrexne\n"
		"strex	%[tmp], %[new], [%[v]]\n"
		: [tmp] "=r" (t), [v] "=r" (v), [old] "=r" (old), [new] "=r" (new)
		: "1" (v), "2" (old), "3" (new)
	);

	return t == 1;
}
