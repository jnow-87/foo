/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <arch/x86/hardware.h>
#include <kernel/init.h>
#include <kernel/interrupt.h>
#include <kernel/sched.h>


/* local/static prototypes */
static void sched_hdlr(int_num_t num, void *payload);


/* local functions */
static int init(void){
	return int_register(INT_SCHED, sched_hdlr, 0x0);
}

platform_init(0, init);

static void sched_hdlr(int_num_t num, void *payload){
	LNX_DEBUG("scheduler interrupt\n");
	sched_tick();
}
