/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/core.h>
#include <kernel/init.h>
#include <kernel/kprintf.h>
#include <sys/escape.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <version.h>


/* local/static prototypes */
static void exec_init_call(init_call_t *base, init_call_t *end);


/* external variables */
extern init_call_t __core_init0_base[],
				   __platform_init0_base[],
				   __platform_init1_base[],
				   __kernel_init0_base[],
				   __kernel_init1_base[],
				   __kernel_init2_base[],
				   __driver_init0_base[],
				   __init_end[];


/* global functions */
int kinit(void){
	/* core (stage: 0) */
	exec_init_call(__core_init0_base, __platform_init0_base);

	if(PIR == 0){
		/* platform (stage: 0, 1) */
		exec_init_call(__platform_init0_base, __kernel_init0_base);

		kprintf(KMSG_DEBUG | KMSG_INFO | KMSG_STAT,
			"\n"
			"\t\t" FG_BLUE "::: brickos :::" RESET_ATTR "\n"
			VERSION "\n"
		);

		/* kernel (stage: 0, 1, 2)
		 * driver (stage: 0)
		 */
		exec_init_call(__kernel_init0_base, __init_end);
	}

	return -errno;
}


/* local functions */
static void exec_init_call(init_call_t *base, init_call_t *end){
	init_call_t *p;


	for(p=base; p<end; p++){
		if(errno != E_OK)
			return;

		DEBUG("%s()...\n", p->name);

		if(p->call() != E_OK)
			FATAL("%s() failed \"%s\"\n", p->name, strerror(errno));
	}
}
