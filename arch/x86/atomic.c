/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



/* global functions */
int x86_cas(int volatile *v, int old, int new){
	asm goto(
		"cmpxchgl	%[new], (%[v])\n"
		"jnz		%l[err]\n"
		:
		: [v] "r" (v), [old] "a" (old), [new] "r" (new)
		: "memory"
		: err
	);

	return 0;


err:
	return 1;
}

void x86_atomic_add(int volatile *v, int inc){
	asm volatile(
		"lock xaddl	%[inc], (%[v])\n"
		: [inc] "+r" (inc)
		: [v] "r" (v)
		: "memory"
	);
}
