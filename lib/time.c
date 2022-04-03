/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/syscall.h>
#include <sys/time.h>
#include <sys/syscall.h>


/* global functions */
int time(time_t *t){
	return sc(SC_TIME, t);
}

uint32_t time_ms(void){
	time_t t;


	if(time(&t) != 0)
		return 0;

	return t.s * 1000 + t.ms;
}
