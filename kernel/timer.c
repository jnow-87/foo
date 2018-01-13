#include <config/config.h>
#include <kernel/init.h>
#include <kernel/kmem.h>
#include <kernel/syscall.h>
#include <kernel/signal.h>
#include <sys/types.h>
#include <sys/list.h>


/* types */
typedef struct timer_t{
	size_t ticks;
	ksignal_t sig;

	struct timer_t *prev,
				   *next;
} timer_t;


/* local/static prototypes */
static int sc_hdlr_sleep(void *p, thread_t const *this_t);
static int sc_hdlr_time(void *p, thread_t const *this_t);

static void time_convert(void);


/* static variables */
static timer_t *timer_lst = 0x0;
static uint32_t time_us = 0;
static time_t time = { 0 };


/* global functions */
void ktimer_tick(void){
	timer_t *t;


	/* increment time */
	time_us += CONFIG_KTIMER_CYCLETIME_US;

	if(time_us + CONFIG_KTIMER_CYCLETIME_US < time_us)
		time_convert();

	/* update timer */
	list_for_each(timer_lst, t){
		t->ticks--;

		if(t->ticks == 0)
			ksignal_send(&t->sig);
	}
}


/* local functions */
static int init(void){
	int e;


	e = 0;
	e |= sc_register(SC_SLEEP, sc_hdlr_sleep);
	e |= sc_register(SC_TIME, sc_hdlr_time);

	return -e;
}

kernel_init(0, init);

static int sc_hdlr_sleep(void *_p, thread_t const *this_t){
	timer_t *t;
	sc_time_t *p;


	p = (sc_time_t*)_p;

	/* allocate timer */
	t = kmalloc(sizeof(timer_t));

	if(t == 0x0)
		goto_errno(err_0, E_NOMEM);

	/* init timer */
	ksignal_init(&t->sig);

	if(p->time.us)		t->ticks = p->time.us / CONFIG_KTIMER_CYCLETIME_US;
	else if(p->time.ms)	t->ticks = p->time.ms / (CONFIG_KTIMER_CYCLETIME_US / 1000);
	else				goto_errno(err_1, E_INVAL);

	if(t->ticks == 0)
		goto_errno(err_1, E_LIMIT);

	list_add_tail(timer_lst, t);

	/* wait for timer to expire */
	(void)ksignal_wait(&t->sig);

	/* cleanup */
	list_rm(timer_lst, t);

err_1:
	kfree(t);

err_0:
	p->errno = errno;
	return E_OK;
}

static int sc_hdlr_time(void *_p, thread_t const *this_t){
	sc_time_t *p;


	p = (sc_time_t*)_p;

	time_convert();

	p->time = time;
	p->errno = E_OK;

	return E_OK;
}

static void time_convert(void){
	uint32_t x;


	/* convert seconds */
	x = time_us / 1000000;
	time_us -= (x * 1000000);

	time.s += x;

	/* convert milliseconds */
	x = time_us / 1000;
	time_us -= (x * 1000);

	time.ms += x;

	/* convert microseconds */
	time.us += time_us;

	time_us = 0;
}
