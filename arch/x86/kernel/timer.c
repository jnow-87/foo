/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <kernel/init.h>
#include <kernel/interrupt.h>
#include <kernel/timer.h>


/* local/static prototypes */
static void timer_hdlr(int_num_t num, void *payload);


/* local functions */
static int init(void){
	return int_register(INT_TIMER, timer_hdlr, 0x0);
}

platform_init(0, init);

static void timer_hdlr(int_num_t num, void *payload){
	LNX_DEBUG("timer tick\n");
	ktimer_tick();
}
