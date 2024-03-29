/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <kernel/stat.h>


/* external variables */
extern stat_call_t __kernel_stat_base[],
				   __kernel_stat_end[];


/* global functions */
void kstat(){
	for(stat_call_t *p=__kernel_stat_base; p<__kernel_stat_end; p++)
		(*p)();
}
