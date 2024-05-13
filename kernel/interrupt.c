/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <kernel/interrupt.h>
#include <kernel/panic.h>
#include <kernel/sched.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/devicetree.h>


/* local/static prototypes */
static void call_hdlr(int_num_t num);
static void foretell(int_num_t num, uint8_t set);


/* static variables */
static int_hdlr_t int_hdlr[DEVTREE_ARCH_NUM_INTS] = { 0x0 };
static void *int_payload[DEVTREE_ARCH_NUM_INTS] = { 0x0 };
static uint8_t foretold[DEVTREE_ARCH_NUM_INTS / 8 + 1] = { 0 };


/* global functions */
int int_register(int_num_t num, int_hdlr_t hdlr, void *payload){
	if(num >= DEVTREE_ARCH_NUM_INTS)
		return_errno(E_INVAL);

	if(int_hdlr[num] != 0x0)
		return_errno(E_INUSE);

	int_hdlr[num] = hdlr;
	int_payload[num] = payload;

	return 0;
}

void int_release(int_num_t num){
	if(num >= DEVTREE_ARCH_NUM_INTS)
		return;

	int_hdlr[num] = 0x0;
	int_payload[num] = 0x0;
}

void int_foretell(int_num_t num){
	foretell(num, 1);
}

void int_khdlr(int_num_t num){
	errno_t e = errno;


	/* handle the given interrupt */
	reset_errno();
	call_hdlr(num);

	/* handle foretold interrupts */
	for(uint8_t i=0; i<sizeof(foretold); i++){
		for(int8_t j=0; j<8 && foretold[i]; j++){
			if(!(foretold[i] & (0x1 << j)))
				continue;

			num = i * 8 + j;

			foretell(num, 0);
			call_hdlr(num);

			// re-iterate the foretold interrupts, handling the case of an
			// interrupt being foretold in a foretold interrupt handler
			i = 0;
			j = -1;
		}
	}

	set_errno(e);
}


/* local functions */
static void call_hdlr(int_num_t num){
	if(num >= DEVTREE_ARCH_NUM_INTS || int_hdlr[num] == 0x0)
		kpanic("unhandled or invalid interrupt %u\n", num);

	int_hdlr[num](num, int_payload[num]);
}

static void foretell(int_num_t num, uint8_t set){
	uint8_t idx,
			bit;


	idx = num / 8;
	bit = num - idx * 8;

	foretold[idx] |= (0x1 << bit);
	foretold[idx] &= ~((set ^ 0x1) << bit);
}
