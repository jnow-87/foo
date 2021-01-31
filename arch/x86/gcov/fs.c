/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>
#include <sys/compiler.h>


/* prototypes */
int gcov_mkdir(const char *path, int mode);
int gc_md(const char *path, int mode) __alias(gcov_mkdir);


/* global functions */
int gcov_mkdir(const char *path, int mode){
	lnx_mkdir(path, mode);

	return 0;
}
