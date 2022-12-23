/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/syscall.h>
#include <sys/syscall.h>
#include <sys/types.h>


/* global functions */
int sleep(size_t ms, size_t us){
	time_t p;


	p.us = us;
	p.ms = ms;

	return sc(SC_SLEEP, &p);
}
