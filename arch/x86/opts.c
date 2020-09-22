/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/opts.h>
#include <lib/init.h>


/* external variables */
extern int argc;
extern char **argv;
extern char **envp;


/* global variables */
x86_opts_t x86_opts = {
	.debug = 0,
	.interactive = false,
};


/* global functions */
int x86_opts_parse(void){
	int i;


	for(i=1; i<argc; i++){
		switch(argv[i][1]){
		case 'v':	x86_opts.debug = 1; break;
		case 'i':	x86_opts.interactive = true; break;
		default:	return -1;
		}
	}

	return 0;
}

lib_init(0, x86_opts_parse);
