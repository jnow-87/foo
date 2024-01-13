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
int avr_atomic(atomic_t op, void *param){
	bool int_en;
	int r;


	/* disable interrupts */
	int_en = mreg_r(SREG) & (0x1 << SREG_I);
	asm volatile("cli");

	r = op(param);

	/* re-enable interrupts */
	if(int_en)
		asm volatile("sei");

	return r;
}
