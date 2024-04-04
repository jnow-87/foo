/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <kernel/driver.h>
#include <kernel/init.h>
#include <kernel/kprintf.h>
#include <kernel/ktask.h>
#include <kernel/opt.h>
#include <kernel/panic.h>
#include <kernel/sched.h>
#include <kernel/stat.h>
#include <kernel/test.h>
#include <sys/errno.h>
#include <sys/escape.h>
#include <sys/string.h>
#include <sys/types.h>
#include <version.h>


/* local/static prototypes */
static void kinit(void);
static void ktask(void);

static void exec_init_call(init_call_t *base, init_call_t *end, bool singular);
static void bootprompt(void);


/* external variables */
extern init_call_t __platform_init0_base_cores_all[],
				   __platform_init0_base_cores_first[],
				   __platform_init1_base_cores_all[],
				   __platform_init1_base_cores_first[],
				   __kernel_init0_base[],
				   __init_end[];


/* global variables */
kopt_t kopt = KOPT_INITIALISER();


/* global functions */
void kernel(void){
	/* init */
	kinit();

	/* kernel statistics and health check */
	if(PIR == 0){
		kstat();
		ktest();
	}

	/* enable interrupts */
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
static void kinit(void){
	/* kernel init */
	// platform (stage: 0, 1)
	exec_init_call(__platform_init0_base_cores_all, __platform_init0_base_cores_first, false);
	exec_init_call(__platform_init0_base_cores_first, __platform_init1_base_cores_all, true);
	exec_init_call(__platform_init1_base_cores_all, __platform_init1_base_cores_first, false);
	exec_init_call(__platform_init1_base_cores_first, __kernel_init0_base, true);

	if(errno != 0)
		kpanic("platform init error: %s\n", strerror(errno));

	bootprompt();

	// kernel (stage: 0, 1, 2)
	exec_init_call(__kernel_init0_base, __init_end, true);

	if(errno != 0)
		kpanic("kernel init error: %s\n", strerror(errno));

	if(PIR != 0)
		return;

	/* driver init */
	if(driver_load() != 0)
		kpanic("driver init error: %s\n", strerror(errno));

	/* multi-core boot */
	cores_boot();
}

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

static void exec_init_call(init_call_t *base, init_call_t *end, bool singular){
	if(singular && PIR != 0)
		return;

	for(init_call_t *p=base; p<end; p++){
		if(errno != 0)
			return;

		DEBUG("%s()...\n", p->name);

		if(p->call() != 0)
			FATAL("init call at %p failed \"%s\"\n", p->call, strerror(errno));
	}
}

static void bootprompt(void){
	if(PIR == 0){
		kprintf(KMSG_ANY,
			"\n"
			"\t\t" FG("::: brickos :::", KOBALT) "\n"
			VERSION "\n"
		);
	}
	else
		kprintf(KMSG_ANY, "starting core %u\n", PIR);
}
