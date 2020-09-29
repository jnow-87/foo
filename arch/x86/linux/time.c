/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>


/* types */
typedef struct{
	long int sec,
			 nsec;
} lnx_timespec_t;


/* global functions */
void lnx_nanosleep(long int ns){
	ssize_t r;
	lnx_timespec_t ts;


	ts.sec = ns / 1000000000;
	ts.nsec = ns % 1000000000;

	while(1){
		r = lnx_syscall(LNX_SYS_NANOSLEEP,
			(unsigned long int[6]){
				(unsigned long int)&ts,
				(unsigned long int)&ts,
				0,
				0,
				0,
				0
			},
			0
		);

		switch(r){
		case 0:
			return;

		case -4: // EINTR
			break;

		default:
			LNX_SYSCALL_ERROR_EXIT("%d\n", r);
		}
	}
}
