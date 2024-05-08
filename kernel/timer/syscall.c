/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/init.h>
#include <kernel/memory.h>
#include <kernel/syscall.h>
#include <kernel/thread.h>
#include <kernel/ksignal.h>
#include <kernel/usignal.h>
#include <kernel/sched.h>
#include <kernel/timer.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mutex.h>
#include <sys/list.h>
#include <sys/signal.h>
#include <sys/time.h>


/* types */
typedef struct timer_t{
	struct timer_t *prev,
				   *next;

	thread_t *thread;
	signal_t sig;

	ktimer_t timer;
} timer_t;


/* local/static prototypes */
static int sc_hdlr_sleep(void *param);
static int sc_hdlr_time(void *param);
static int sc_hdlr_timer(void *param);

static void timer_hdlr(void *payload);
static void timer_thread_dtor(thread_t *this_t, void *payload);

static int timer_create(signal_t sig, uint32_t period_us);
static void timer_destroy(timer_t *timer);


/* static variables */
static timer_t *timers = 0x0;
static mutex_t timers_mtx = MUTEX_INITIALISER();


/* local functions */
static int init(void){
	int r = 0;


	r |= sc_register(SC_SLEEP, sc_hdlr_sleep);
	r |= sc_register(SC_TIME, sc_hdlr_time);
	r |= sc_register(SC_TIMER, sc_hdlr_timer);

	return r;
}

kernel_init(0, init);

static int sc_hdlr_sleep(void *param){
	time_t *p = (time_t*)param;
	int r;
	ksignal_t sig;
	mutex_t mtx;


	ksignal_init(&sig);
	mutex_init(&mtx, MTX_NOINT);

	mutex_lock(&mtx);
	r = ksignal_timedwait(&sig, &mtx, p->us ? p->us : ((uint32_t)p->ms * 1000));
	mutex_unlock(&mtx);

	return r;
}

static int sc_hdlr_time(void *param){
	ktimer_time(param);

	return 0;
}

static int sc_hdlr_timer(void *param){
	sc_timer_t *p = (sc_timer_t*)param;
	timer_t *timer;


	if(p->period_us != 0)
		return timer_create(p->sig, p->period_us);

	timer = list_find_safe(timers, sig, p->sig, &timers_mtx);

	if(timer == 0x0)
		return_errno(E_INVAL);

	timer_destroy(timer);

	return 0;
}

static void timer_hdlr(void *payload){
	timer_t *timer = (timer_t*)payload;


	usignal_send(timer->thread, timer->sig);
}

static void timer_thread_dtor(thread_t *this_t, void *payload){
	timer_destroy(payload);
}

static int timer_create(signal_t sig, uint32_t period_us){
	thread_t *this_t;
	timer_t *timer;


	this_t = sched_running();
	timer = kmalloc(sizeof(timer_t));

	if(timer == 0x0)
		goto err_0;

	timer->thread = this_t;
	timer->sig = sig;

	ktimer_start(&timer->timer, period_us, timer_hdlr, timer, true);

	if(thread_dtor_register(this_t, timer_thread_dtor, timer) != 0)
		goto err_1;

	list_add_tail_safe(timers, timer, &timers_mtx);

	return 0;


err_1:
	timer_destroy(timer);

err_0:
	return -errno;
}

static void timer_destroy(timer_t *timer){
	thread_dtor_release(timer->thread, timer_thread_dtor, timer);
	ktimer_abort(&timer->timer);
	kfree(timer);
}
