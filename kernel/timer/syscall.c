/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <kernel/init.h>
#include <kernel/syscall.h>
#include <kernel/ksignal.h>
#include <kernel/timer.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/mutex.h>
#include <sys/time.h>


/* local/static prototypes */
static int sc_hdlr_sleep(void *p);
static int sc_hdlr_time(void *p);


/* local functions */
static int init(void){
	int r;


	r = 0;

	r |= sc_register(SC_SLEEP, sc_hdlr_sleep);
	r |= sc_register(SC_TIME, sc_hdlr_time);

	return r;
}

kernel_init(0, init);

static int sc_hdlr_sleep(void *_p){
	int r;
	ksignal_t sig;
	mutex_t mtx;
	time_t *p;


	p = (time_t*)_p;
	ksignal_init(&sig);
	mutex_init(&mtx, MTX_NOINT);

	mutex_lock(&mtx);
	r = ksignal_timedwait(&sig, &mtx, p->us ? p->us : ((uint32_t)p->ms * 1000));
	mutex_unlock(&mtx);

	return r;
}

static int sc_hdlr_time(void *p){
	ktimer_time(p);

	return 0;
}
