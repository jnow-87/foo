/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/interrupt.h>
#include <kernel/init.h>
#include <kernel/panic.h>
#include <kernel/thread.h>
#include <kernel/sched.h>
#include <sys/register.h>


/* local/static prototypes */
static void iovfl_hdlr(int_num_t num, void *payload);


/* local functions */
static int init(void){
	return int_register(AVR_NUM_HW_INTS + 1, iovfl_hdlr, 0x0);
}

platform_init(0, init);

static void iovfl_hdlr(int_num_t num, void *payload){
	kpanic("instruction memory overflow\n");
}
