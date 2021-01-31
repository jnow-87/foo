/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <arch/interrupt.h>
#include <kernel/interrupt.h>
#include <kernel/panic.h>
#include <kernel/sched.h>
#include <sys/errno.h>


/* static variables */
static int_hdlr_t int_hdlr[NUM_INT] = { 0x0 };
static void *int_data[NUM_INT] = { 0x0 };


/* global functions */
int int_register(int_num_t num, int_hdlr_t hdlr, void *data){
	if(num >= NUM_INT)
		return_errno(E_INVAL);

	if(int_hdlr[num] != 0x0)
		return_errno(E_INUSE);

	int_hdlr[num] = hdlr;
	int_data[num] = data;

	return E_OK;
}

void int_release(int_num_t num){
	if(num >= NUM_INT)
		return;

	int_hdlr[num] = 0x0;
	int_data[num] = 0x0;
}

void int_call(int_num_t num){
	int_type_t imask;


	imask = int_enable(INT_NONE);

	if(num >= NUM_INT || int_hdlr[num] == 0x0)
		kpanic("unhandled or invalid interrupt %u\n", num);

	errno = E_OK;
	int_hdlr[num](num, int_data[num]);

	int_enable(imask);
}
