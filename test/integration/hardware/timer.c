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


/* global functions */
void hw_timer(void){
	struct timespec ts;


	ts.tv_sec = CONFIG_KTIMER_CYCLETIME_US / 1000000;
	ts.tv_nsec = (CONFIG_KTIMER_CYCLETIME_US % 1000000) * 1000;

	while(nanosleep(&ts, &ts) && errno == EINTR);

	hw_int_request(INT_TIMER, 0x0, PRIV_HARDWARE, 0);
}
