/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/interrupt.h>
#include <kernel/opt.h>
#include <kernel/panic.h>
#include <kernel/sched.h>
#include <kernel/ktask.h>
#include <kernel/driver.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <sys/list.h>
#include <sys/types.h>
#include "kernel.h"


/* local/static prototypes */
static void ktask(void);


/* global variables */
kopt_t kopt = KOPT_INITIALISER();


/* global functions */
void kernel(void){
	/* init */
	if(kinit() != 0)
		kpanic("kernel init error \"%s\"\n", strerror(errno));

	if(PIR == 0 && driver_load() != 0)
		kpanic("driver init error \"%s\"\n", strerror(errno));

	/* kernel statistics and health check*/
	if(PIR == 0){
		kstat();
		ktest();
	}

	/* enable interrupts */
	// TODO without the barrier core0 panics and the output
	// doesn't match what is supposed to happen, e.g. core1
	// seems to do driver initialisation
//	INFO("smp barrier\n");
//	atomic_inc(&ncores, 1);
//
//	while(ncores != DEVTREE_ARCH_NCORES);
//
//	INFO("smp barrier passed\n");

	int_enable(true);

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
	ktask_t *rec_task = 0x0;
	ktask_t *task;


	while(1){
		task = ktask_next();

		/* ensure recurring tasks are only executed once in this loop */
		if(task && task == rec_task)
			ktask_complete(task);

		/* break */
		if(task == 0x0 || task == rec_task)
			return;

		/* remember first recurring task */
		if((task->flags & TASK_RECURRING) && rec_task == 0x0)
			rec_task = task;

		/* execute task */
		task->hdlr(task->payload);
		ktask_complete(task);
	}
}
