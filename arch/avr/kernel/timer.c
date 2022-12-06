/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <config/avrconfig.h>
#include <arch/arch.h>
#include <kernel/interrupt.h>
#include <kernel/init.h>
#include <kernel/stat.h>
#include <kernel/sched.h>
#include <kernel/timer.h>
#include <kernel/kprintf.h>
#include <sys/errno.h>
#include <sys/const.h>


/* macros */
#define STRVAL(s) STR(s)


/* local/static prototypes */
static void timer_hdlr(int_num_t num, void *payload);


/* local functions */
static int init(void){
	/* clear reset flags */
	mreg_w(MCUSR, 0x0);

	/* set prescale value and interrupt */
	// start timed sequence
	mreg_w(WDTCSR,
		(0x1 << WDTCSR_WDCE) |
		(0x1 << WDTCSR_WDE)
	);

	// set config
	mreg_w(WDTCSR,
		(0x1 << WDTCSR_WDIE) |

#if (AVRCONFIG_WATCHDOG_PRESCALE == _2k)
		0x0
#elif (AVRCONFIG_WATCHDOG_PRESCALE == _4k)
		(0x1 << WDTCSR_WDP0)
#elif (AVRCONFIG_WATCHDOG_PRESCALE == _8k)
		(0x2 << WDTCSR_WDP0)
#elif (AVRCONFIG_WATCHDOG_PRESCALE == _16k)
		(0x3 << WDTCSR_WDP0)
#elif (AVRCONFIG_WATCHDOG_PRESCALE == _32k)
		(0x4 << WDTCSR_WDP0)
#elif (AVRCONFIG_WATCHDOG_PRESCALE == _64k)
		(0x5 << WDTCSR_WDP0)
#elif (AVRCONFIG_WATCHDOG_PRESCALE == _128k)
		(0x6 << WDTCSR_WDP0)
#elif (AVRCONFIG_WATCHDOG_PRESCALE == _256k)
		(0x7 << WDTCSR_WDP0)
#elif (AVRCONFIG_WATCHDOG_PRESCALE == _512k)
		(0x1 << WDTCSR_WDP3)
#elif (AVRCONFIG_WATCHDOG_PRESCALE == _1M)
		(0x1 << WDTCSR_WDP3) |
		(0x1 << WDTCSR_WDP0)
#endif // AVRCONFIG_WATCHDOG_PRESCALE
	);

	return int_register(CONFIG_TIMER_INT, timer_hdlr, 0x0);
}

platform_init(0, init);

static void stats(void){
#ifdef CONFIG_KTIMER_CYCLETIME_US
	STAT("kernel time base: " STRVAL(CONFIG_KTIMER_CYCLETIME_US) "us\n");
	STAT("kernel time base error: " STRVAL(AVRCONFIG_KTIMER_ERROR_US) "us\n");
#endif // CONFIG_KTIMER_CYCLETIME_US
}

kernel_stat(stats);

static void timer_hdlr(int_num_t num, void *payload){
	static size_t timer = 0;


	timer++;

	/* trigger kernel timer */
	if(timer == AVRCONFIG_KTIMER_FACTOR){
		timer = 0;
		ktimer_tick();
	}
}
