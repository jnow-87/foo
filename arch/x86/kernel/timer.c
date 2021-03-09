/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/x86/linux.h>
#include <arch/x86/hardware.h>
#include <kernel/init.h>
#include <kernel/interrupt.h>
#include <kernel/sched.h>
#include <kernel/timer.h>
#include <sys/types.h>
#include <sys/math.h>


/* macros */
#define CYCLE_TIME_US	MIN(CONFIG_KTIMER_CYCLETIME_US, CONFIG_SCHED_CYCLETIME_US)
#define TIMER_FACTOR 	(CONFIG_KTIMER_CYCLETIME_US / CYCLE_TIME_US)
#define SCHED_FACTOR	(CONFIG_SCHED_CYCLETIME_US / CYCLE_TIME_US)


/* local functions */
static void timer_hdlr(int_num_t num, void *data){
	static size_t timer = 0,
				  sched = 0;


	timer++;
	sched++;

	LNX_DEBUG("timer interrupt\n");

	/* trigger kernel timer */
#ifdef CONFIG_KERNEL_TIMER
	if(timer == TIMER_FACTOR){
		LNX_DEBUG("timer tick\n");
		timer = 0;
		ktimer_tick();
	}
#endif // CONFIG_KERNEL_TIMER

	/* trigger scheduler */
#ifdef CONFIG_SCHED_PREEMPTIVE
	if(sched == SCHED_FACTOR){
		LNX_DEBUG("sched tick\n");
		sched = 0;
		sched_tick();
	}
#endif // CONFIG_SCHED_PREEMPTIVE
}

static int init(void){
	return int_register(INT_TIMER, timer_hdlr, 0x0);
}

platform_init(0, init);
