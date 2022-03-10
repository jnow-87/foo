/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>


/* global functions */
void *lnx_mmap(void *addr, size_t len, int prot, int flags, int fd, unsigned long int offset){
	return (void*)lnx_syscall(LNX_SYS_MMAP,
		(unsigned long int[6]){
			(unsigned long int)addr,
			len,
			prot,
			flags,
			fd,
			offset
		},
		1
	);
}
