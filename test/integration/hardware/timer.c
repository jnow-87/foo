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
#define CYCLE_TIME_US	MIN(CONFIG_KTIMER_CYCLETIME_US, CONFIG_SCHED_CYCLETIME_US)
#define TIMER_FACTOR 	(CONFIG_KTIMER_CYCLETIME_US / CYCLE_TIME_US)
#define SCHED_FACTOR	(CONFIG_SCHED_CYCLETIME_US / CYCLE_TIME_US)


/* global functions */
void hw_timer(void){
	static size_t timer = 0;
	struct timespec ts;


	ts.tv_sec = CYCLE_TIME_US / 1000000;
	ts.tv_nsec = (CYCLE_TIME_US % 1000000) * 1000;

	while(nanosleep(&ts, &ts) && errno == EINTR);

	if(++timer == TIMER_FACTOR){
		hw_int_request(INT_TIMER, 0x0, PRIV_HARDWARE, 0);
		timer = 0;
	}
}

void hw_sched_timer(void){
	static size_t timer = 0;
	struct timespec ts;


	ts.tv_sec = CYCLE_TIME_US / 1000000;
	ts.tv_nsec = (CYCLE_TIME_US % 1000000) * 1000;

	while(nanosleep(&ts, &ts) && errno == EINTR);

	if(++timer == SCHED_FACTOR){
		hw_int_request(INT_SCHED, 0x0, PRIV_HARDWARE, 0);
		timer = 0;
	}
}
