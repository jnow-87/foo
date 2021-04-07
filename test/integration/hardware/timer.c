/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <time.h>
#include <errno.h>
#include <sys/math.h>
#include <user/debug.h>
#include <hardware/hardware.h>


/* macros */
#define TIMER_US	MIN(CONFIG_KTIMER_CYCLETIME_US, CONFIG_SCHED_CYCLETIME_US)


/* global functions */
void hw_timer(void){
	struct timespec ts;


	ts.tv_sec = TIMER_US / 1000000;
	ts.tv_nsec = (TIMER_US % 1000000) * 1000;

	while(nanosleep(&ts, &ts) && errno == EINTR);

	DEBUG(0, "enqueue timer interrupt\n");
	hw_int_request(INT_TIMER, 0x0, HWS_HARDWARE, 0);
}
