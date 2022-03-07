/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/init.h>
#include <kernel/memory.h>
#include <kernel/syscall.h>
#include <kernel/ksignal.h>
#include <sys/types.h>
#include <sys/list.h>
#include <sys/mutex.h>


/* macros */
#define CYCLE_TIME_US	((uint32_t)(CONFIG_KTIMER_CYCLETIME_US + arch_info(kernel_timer_err_us)))


/* types */
typedef struct timer_t{
	struct timer_t *prev,
				   *next;

	size_t ticks;
	ksignal_t sig;
} timer_t;


/* local/static prototypes */
static int sc_hdlr_sleep(void *p);
static int sc_hdlr_time(void *p);

static size_t to_ticks(uint32_t us);
static void to_time(void);


/* static variables */
static timer_t *timer_lst = 0x0;
static mutex_t timer_mtx = NOINT_MUTEX_INITIALISER();
static uint32_t time_us = 0;
static time_t time = { 0 };


/* global functions */
void ktimer_tick(void){
	timer_t *t;


	/* increment time */
	time_us += CYCLE_TIME_US;

	if(time_us + CYCLE_TIME_US + 1001000 < time_us)
		to_time();

	/* update timer */
	mutex_lock(&timer_mtx);

	list_for_each(timer_lst, t){
		t->ticks--;

		if(t->ticks == 0)
			ksignal_send(&t->sig);
	}

	mutex_unlock(&timer_mtx);
}


/* local functions */
static int init(void){
	int r;


	r = E_OK;

	r |= sc_register(SC_SLEEP, sc_hdlr_sleep);
	r |= sc_register(SC_TIME, sc_hdlr_time);

	return r;
}

kernel_init(0, init);

static int sc_hdlr_sleep(void *_p){
	size_t ticks;
	timer_t t;
	time_t *p;


	p = (time_t*)_p;

	/* compute ticks */
	if(p->us)		ticks = to_ticks(p->us);
	else if(p->ms)	ticks = to_ticks((uint32_t)p->ms * 1000);
	else			return_errno(E_INVAL);

	if(ticks == 0)
		return_errno(E_LIMIT);

	/* init timer */
	t.ticks = ticks;
	ksignal_init(&t.sig);

	mutex_lock(&timer_mtx);

	list_add_tail(timer_lst, &t);
	ksignal_wait(&t.sig, &timer_mtx);
	list_rm(timer_lst, &t);

	mutex_unlock(&timer_mtx);

	return E_OK;
}

static int sc_hdlr_time(void *_p){
	time_t *p;


	p = (time_t*)_p;

	to_time();
	*p = time;

	return E_OK;
}

static size_t to_ticks(uint32_t us){
	size_t ticks;


	ticks = us / CYCLE_TIME_US;

	if(us - ticks * CYCLE_TIME_US > CYCLE_TIME_US / 10)
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
