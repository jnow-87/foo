/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>
#include <kernel/init.h>
#include <kernel/kprintf.h>
#include <sys/escape.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <version.h>


/* local/static prototypes */
static void exec_init_call(init_call_t *base, init_call_t *end, bool singular);
static void bootprompt(void);


/* external variables */
extern init_call_t __platform_init0_base_cores_all[],
				   __platform_init0_base_cores_first[],
				   __platform_init1_base_cores_all[],
				   __platform_init1_base_cores_first[],
				   __kernel_init0_base[],
				   __init_end[];


/* global functions */
int kinit(void){
	// platform (stage: 0, 1)
	exec_init_call(__platform_init0_base_cores_all, __platform_init0_base_cores_first, false);
	exec_init_call(__platform_init0_base_cores_first, __platform_init1_base_cores_all, true);
	exec_init_call(__platform_init1_base_cores_all, __platform_init1_base_cores_first, false);
	exec_init_call(__platform_init1_base_cores_first, __kernel_init0_base, true);

	bootprompt();

	// kernel (stage: 0, 1, 2)
	exec_init_call(__kernel_init0_base, __init_end, true);

	return -errno;
}


/* local functions */
static void exec_init_call(init_call_t *base, init_call_t *end, bool singular){
	if(singular && PIR != 0)
		return;

	for(init_call_t *p=base; p<end; p++){
		if(errno != 0)
			return;

		DEBUG("%s()...\n", p->name);

		if(p->call() != 0)
			FATAL("%s() failed \"%s\"\n", p->name, strerror(errno));
	}
}

static void bootprompt(void){
	if(PIR == 0){
		kprintf(KMSG_ANY,
			"\n"
			"\t\t" FG_BLUE "::: brickos :::" RESET_ATTR "\n"
			VERSION "\n"
		);
	}
	else
		kprintf(KMSG_ANY, "starting core %u\n", PIR);
}
