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


/* static variables */
timer_t *timer_lst = 0x0;


/* global functions */
void ktimer_tick(void){
	timer_t *t;


	list_for_each(timer_lst, t){
		t->ticks--;

		if(t->ticks == 0)
			ksignal_send(&t->sig);
	}
}


/* local functions */
static int init(void){
	return sc_register(SC_SLEEP, sc_hdlr_sleep);
}

kernel_init(0, init);

static int sc_hdlr_sleep(void *_p, thread_t const *this_t){
	timer_t *t;
	sc_sleep_t *p;


	p = (sc_sleep_t*)_p;

	/* allocate timer */
	t = kmalloc(sizeof(timer_t));

	if(t == 0x0)
		goto_errno(err_0, E_NOMEM);

	/* init timer */
	ksignal_init(&t->sig);

	if(p->us)		t->ticks = p->us / CONFIG_KTIMER_CYCLETIME_US;
	else if(p->ms)	t->ticks = p->ms / (CONFIG_KTIMER_CYCLETIME_US / 1000);
	else			goto_errno(err_1, E_INVAL);

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
