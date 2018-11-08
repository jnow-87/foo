/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/syscall.h>
#include <sys/syscall.h>


/* global functions */
void sched_yield(void){
	char dummy;


	(void)sc(SC_SCHEDYIELD, &dummy);
}
