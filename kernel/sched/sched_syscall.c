#include <kernel/init.h>
#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <sys/syscall.h>
#include <sys/errno.h>


/* local/static prototypes */
static int sc_hdlr_sched_yield(void *p);


/* local functions */
static int init(void){
	sc_register(SC_SCHEDYIELD, sc_hdlr_sched_yield);

	return -errno;
}

kernel_init(2, init);

static int sc_hdlr_sched_yield(void *p){
	sched_tick();
	return E_OK;
}
