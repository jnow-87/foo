/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <sys/compiler.h>


/* prototypes */
void gcov_abort(void);
void gc_at(void) __alias(gcov_abort);

int *gcov___errno_location(void);
int *gc_errno_loc____(void) __alias(gcov___errno_location);


/* global functions */
void gcov_abort(void){
	LNX_EEXIT("not expected to be called\n");
}

int * gcov___errno_location(void){
	static int gcov_errno = 0;


	return &gcov_errno;
}
