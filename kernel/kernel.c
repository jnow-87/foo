#include <arch/interrupt.h>
#include <kernel/opt.h>
#include <kernel/panic.h>
#include <kernel/sched.h>
#include <kernel/task.h>
#include <sys/errno.h>
#include <sys/list.h>
#include "kernel.h"


/* local/static prototypes */
static void ktask(void);


/* global variables */
kopt_t kopt = KOPT_INITIALISER();


/* global functions */
void kernel(void){
	/* init */
	if(kinit() < 0)
		kpanic(0x0, "error (%#x) during kernel init", errno);

	/* kernel statistics */
	kstat();

	/* kernel test */
	ktest();

	/* enable interrupts */
	int_enable(INT_ALL);

	/* kernel thread */
	while(1){
		// handle kernel tasks
		ktask();

		// suspend
		sched_yield();
	}
}


/* local functions */
static void ktask(void){
	ktask_t *task;


	while(1){
		task = ktask_next();

		if(task == 0x0)
			return;

		task->hdlr(task->data);
		ktask_destroy(task);
	}
}
