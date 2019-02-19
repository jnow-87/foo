/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/types.h>


/* external prototypes */
void kernel(void);


/* external variables */
extern uint32_t __data_start[],
				__data_end[],
				__data_load_start[],
				__data_load_end[],
				__bss_start[],
				__bss_end[];


/* global functions */
void __start(void){
	uint32_t *s,
			 *d;


	/* init .data */
	for(s=__data_load_start, d=__data_start; s<=__data_load_end; s++, d++)
		*d = *s;

	/* init .bss */
	for(d=__bss_start; d<=__bss_end; d++)
		*d = 0x0;

	/* call kernel */
	kernel();
}
