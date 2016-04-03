#include <arch/interrupt.h>
#include <kernel/init.h>
#include <sys/error.h>


/* local/static prototypes */
static error_t sched_tick(int_num_t num);


/* global variables */
thread_t* current_thread[CONFIG_NCORES];


/* static variables */
// TODO scheduler lists


/* local functions */
static error_t sched_init(void){
	// TODO init scheduler lists
	// TODO register sched_tick as timer interrupt handler
	if(int_hdlr_register(INT_SCHED, sched_tick) != E_OK)
		return E_INUSE;

	// TODO load init application
	return E_OK;
}

kernel_init(2, sched_init);


static error_t sched_tick(int_num_t num){
	// TODO check for next thread
	// TODO switch thread or goto sleep
	return E_OK;
}
