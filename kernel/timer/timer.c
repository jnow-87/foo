/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/arch.h>
#include <kernel/init.h>
#include <kernel/timer.h>
#include <kernel/interrupt.h>
#include <kernel/sched.h>
#include <sys/types.h>
#include <sys/list.h>
#include <sys/mutex.h>
#include <sys/devicetree.h>


/* local/static prototypes */
static void timer_hdlr(int_num_t num, void *payload);

static size_t to_ticks(uint32_t us);
static void to_time(void);


/* static variables */
#ifdef CONFIG_SCHED_PREEMPTIVE
static size_t sched_ticks = 0;
#endif // CONFIG_SCHED_PREEMPTIVE

static ktimer_t *timer_lst = 0x0;
static mutex_t timer_mtx = NOINT_MUTEX_INITIALISER();

static uint32_t time_us = 0;
static time_t time = { 0 };


/* global functions */
void ktimer_register(ktimer_t *timer, uint32_t cycle_time_us, ktimer_hdlr_t hdlr, void *payload, bool periodic){
	timer->hdlr = hdlr;
	timer->payload = payload;
	timer->ticks = to_ticks(cycle_time_us);
	timer->base = periodic ? timer->ticks : 0;

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

uint32_t ktimer_ms(void){
	to_time();

	return time.s * 1000 + time.ms;
}

void ktimer_time(time_t *t){
	to_time();
	*t = time;
}


/* local functions */
static int init(void){
	return int_register(DEVTREE_ARCH_TIMER_INT, timer_hdlr, 0x0);
}

platform_init(0, init);

static void timer_hdlr(int_num_t num, void *payload){
	ktimer_t *t;


	/* trigger scheduler */
#ifdef CONFIG_SCHED_PREEMPTIVE
	sched_ticks++;

	if(sched_ticks >= CONFIG_SCHED_KTIMER_MUL){
		sched_ticks = 0;
		sched_trigger();
	}
#endif // CONFIG_SCHED_PREEMPTIVE

	/* increment time */
	time_us += DEVTREE_ARCH_TIMER_CYCLE_TIME_US;

	if(time_us + DEVTREE_ARCH_TIMER_CYCLE_TIME_US + 1001000 < time_us)
		to_time();

	/* update timer */
	mutex_lock(&timer_mtx);

	list_for_each(timer_lst, t){
		if(--t->ticks == 0){
			t->hdlr(t->payload);
			t->ticks = t->base;

			if(t->base == 0)
				list_rm(timer_lst, t);
		}
	}

	mutex_unlock(&timer_mtx);
}

static size_t to_ticks(uint32_t us){
	size_t ticks = us / DEVTREE_ARCH_TIMER_CYCLE_TIME_US;


	if(ticks == 0 || us - ticks * DEVTREE_ARCH_TIMER_CYCLE_TIME_US > 0)
		ticks++;

	return ticks;
}

static void to_time(void){
	mutex_lock(&timer_mtx);

	time_us += (uint32_t)time.ms * 1000 + time.us;

	time.us = time_us % 1000;
	time.ms = (time_us / 1000) % 1000;
	time.s += time_us / 1000000;

	time_us = 0;

	mutex_unlock(&timer_mtx);
}
