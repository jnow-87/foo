/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/signal.h>


/* global functions */
int timer_register(signal_t sig, uint32_t period_us){
	sc_timer_t p;


	p.sig = sig;
	p.period_us = period_us;

	return sc(SC_TIMER, &p);
}

void timer_release(signal_t sig){
	(void)timer_register(sig, 0);
}
