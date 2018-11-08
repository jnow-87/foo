/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <arch/avr/register.h>
#include <sys/types.h>


/* global functions */
int avr_cas(int volatile *v, int old, int new){
	bool int_en;
	int t;


	/* disable interrupts */
	int_en = mreg_r(SREG) & (0x1 << SREG_I);
	asm volatile("cli");

	/* compare and swap */
	t = *v;

	if(t == old)
		*v = new;

	/* enable interrupts */
	if(int_en)
		asm volatile("sei");

	return t != old;
}
