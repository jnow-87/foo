/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/panic.h>
#include <kernel/thread.h>
#include <kernel/sched.h>
#include <sys/register.h>


/* global functions */
void avr_iovfl_hdlr(struct thread_context_t *tc){
	unsigned int ret_addr;


	ret_addr = ((lo8(tc->ret_addr) << 8) | hi8(tc->ret_addr)) * 2;
	kpanic(sched_running(), "instruction memory overflow at 0x%x", ret_addr);
}
