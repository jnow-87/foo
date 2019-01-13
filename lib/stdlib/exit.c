/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/syscall.h>
#include <lib/stdlib.h>
#include <sys/syscall.h>


/* global functions */
void exit(int status){
	_exit(status, true);
}

void _exit(int status, bool kill_sibl){
	sc_exit_t p;


	p.status = status;
	p.kill_siblings = kill_sibl;

	(void)sc(SC_EXIT, &p);
}
