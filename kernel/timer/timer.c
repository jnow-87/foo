/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <kernel/timer.h>
#include <sys/types.h>
#include <sys/list.h>
#include <sys/mutex.h>


/* macros */
#define CYCLE_TIME_US	((uint32_t)(CONFIG_KTIMER_CYCLETIME_US + arch_info(kernel_timer_err_us)))


/* local/static prototypes */
static size_t to_ticks(uint32_t us);
static void to_time(void);


/* static variables */
static ktimer_t *timer_lst = 0x0;
static mutex_t timer_mtx = NOINT_MUTEX_INITIALISER();

static uint32_t time_us = 0;
static time_t time = { 0 };


/* global functions */
void ktimer_tick(void){
	ktimer_t *t;


	/* increment time */
	time_us += CYCLE_TIME_US;

	if(time_us + CYCLE_TIME_US + 1001000 < time_us)
		to_time();

	/* update timer */
	mutex_lock(&timer_mtx);

	list_for_each(timer_lst, t){
		if(--t->ticks == 0){
			t->hdlr(t->data);
			list_rm(timer_lst, t);
		}
	}

	mutex_unlock(&timer_mtx);
}

void ktimer_register(ktimer_t *timer, uint32_t period_us, ktimer_hdlr_t hdlr, void *data){
	timer->hdlr = hdlr;
	timer->data = data;
	timer->ticks = to_ticks(period_us);

	mutex_lock(&timer_mtx);
	list_add_tail(timer_lst, timer);
	mutex_unlock(&timer_mtx);
}

void ktimer_release(ktimer_t *timer){
	mutex_lock(&timer_mtx);

	if(timer->ticks != 0)
		list_rm(timer_lst, timer);

	mutex_unlock(&timer_mtx);
}

void ktimer_time(time_t *t){
	to_time();
	*t = time;
}


/* local functions */
static size_t to_ticks(uint32_t us){
	size_t ticks;


	ticks = us / CYCLE_TIME_US;

	if(ticks == 0 || us - ticks * CYCLE_TIME_US > 0)
		ticks++;

	return ticks;
}

static void to_time(void){
	mutex_lock(&timer_mtx);

	time_us += time.ms * 1000 + time.us;

	time.us = time_us % 1000;
	time.ms = (time_us / 1000) % 1000;
	time.s += time_us / 1000000;

	time_us = 0;

	mutex_unlock(&timer_mtx);
}
