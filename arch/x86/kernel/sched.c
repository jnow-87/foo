/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/interrupt.h>
#include <arch/x86/linux.h>
#include <kernel/thread.h>
#include <kernel/sched.h>


/* global functions */
void x86_sched_trigger(void){
	int_type_t imask;


	imask = int_enabled();
	sched_trigger();
	int_enable(imask);	// disabled by sched_trigger()
}

void x86_sched_yield(void){
	thread_t *this_t;


	this_t = sched_running();

	while(sched_running() == this_t){
		lnx_pause();
	}
}

void x86_sched_wait(thread_t *this_t){
	while(sched_running() != this_t){
		lnx_pause();
	}
}

void x86_sched_force(thread_t *this_t){
	if(this_t->state != READY)
		return;

	while(sched_running() != this_t){
		x86_sched_trigger();
	}
}
