/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <sys/compiler.h>


/* prototypes */
char *gcov_getenv(char const *name);
char *gc_gev(char const *name) __alias(gcov_getenv);

int gcov_getpid(void);
int gc_pid(void) __alias(gcov_getpid);


/* global functions */
int gcov_getpid(void){
	return lnx_getpid();
}

char *gcov_getenv(char const *name){
	return 0x0;
}
