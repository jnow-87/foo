/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <arch/interrupt.h>
#include <kernel/interrupt.h>
#include <kernel/panic.h>
#include <kernel/sched.h>
#include <sys/types.h>
#include <sys/errno.h>


/* macros */
#define NUM_INTS	(ARCH_NUM_INTS + CONFIG_INT_VIRTUALS)


/* local/static prototypes */
static void call_hdlr(int_num_t num);
static void foretell(int_num_t num, uint8_t set);


/* static variables */
static int_hdlr_t int_hdlr[NUM_INTS] = { 0x0 };
static void *int_data[NUM_INTS] = { 0x0 };
static uint8_t foretold[NUM_INTS / 8 + 1] = { 0 };


/* global functions */
int int_register(int_num_t num, int_hdlr_t hdlr, void *data){
	if(num >= NUM_INTS)
		return_errno(E_INVAL);

	if(int_hdlr[num] != 0x0)
		return_errno(E_INUSE);

	int_hdlr[num] = hdlr;
	int_data[num] = data;

	return E_OK;
}

void int_release(int_num_t num){
	if(num >= NUM_INTS)
		return;

	int_hdlr[num] = 0x0;
	int_data[num] = 0x0;
}

void int_foretell(int_num_t num){
	foretell(num, 1);
}

void int_khdlr(int_num_t num){
	uint8_t i,
			j;
	errno_t e;


	e = errno;
	errno = E_OK;

	/* handle the given interrupt */
	call_hdlr(num);

	/* handle foretold interrupts */
	for(i=0; i<sizeof(foretold); i++){
		for(j=0; j<8 && foretold[i]; j++){
			if(!(foretold[i] & (0x1 << j)))
				continue;

			num = i * 8 + j;

			foretell(num, 0);
			call_hdlr(num);
		}
	}

	errno = e;
}


/* local functions */
static void call_hdlr(int_num_t num){
	if(num >= NUM_INTS || int_hdlr[num] == 0x0)
		kpanic("unhandled or invalid interrupt %u\n", num);

	int_hdlr[num](num, int_data[num]);
}

static void foretell(int_num_t num, uint8_t set){
	uint8_t idx,
			bit;


	idx = num / 8;
	bit = num - idx * 8;

	foretold[idx] |= (0x1 << bit);
	foretold[idx] &= ~((set ^ 0x1) << bit);
}
