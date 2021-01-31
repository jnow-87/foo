/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/opts.h>
#include <stdio.h>
#include <sched.h>


/* global functions */
int main(int argc, char **argv){
	if(!x86_opts.interactive){
		printf("hello world\n");

		sched_yield();
	}
	else{
		printf("app in interactive mode\n");

		while(1){
			sched_yield();
		}
	}

	return 0;
}
