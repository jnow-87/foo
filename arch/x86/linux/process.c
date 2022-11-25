/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>


/* global functions */
long int lnx_getpid(void){
	static long int pid = 0;


	if(pid == 0)
		pid = lnx_syscall(LNX_SYS_GETPID, (unsigned long int[6]){0, 0, 0, 0, 0, 0}, 1);

	if(pid <= 0)
		LNX_SYSCALL_ERROR_EXIT("%d\n", pid);

	return pid;
}

long int lnx_getppid(void){
	static long int pid = 0;


	if(pid == 0)
		pid = lnx_syscall(LNX_SYS_GETPPID, (unsigned long int[6]){0, 0, 0, 0, 0, 0}, 1);

	if(pid <= 0)
		LNX_SYSCALL_ERROR_EXIT("%d\n", pid);

	return pid;
}

void lnx_exit(int code){
	(void)lnx_syscall(LNX_SYS_EXIT, (unsigned long int[6]){code, 0, 0, 0, 0, 0}, 0);
	LNX_ERROR("lnx_exit failed\n");

	while(1){
		lnx_pause();
	}
}
