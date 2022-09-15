/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/x86/linux.h>


/* macros */
#define IPC_CREAT	512
#define IPC_RMID	0


/* global functions */
long int lnx_shmget(size_t n){
	return lnx_syscall(LNX_SYS_SHMGET,
		(unsigned long int[6]){
			0,
			n,
			IPC_CREAT | 0600,
			0,
			0,
			0
		},
		1
	);
}

void *lnx_shmat(int id){
	long int p;


	p = lnx_syscall(LNX_SYS_SHMAT,
		(unsigned long int[6]){
			id,
			0x0,
			0,
			0,
			0,
			0
		},
		1
	);

	switch(p){
	case -LNX_EACCESS:	// fall through
	case -LNX_EIDRM:	// fall through
	case -LNX_EINVAL:	// fall through
	case -LNX_ENOMEM:	// fall through
		return (void*)-1;
	}

	return (void*)p;
}

void lnx_shmrm(int id, void *addr){
	if(addr != 0x0 && addr != (void*)-1){
		lnx_syscall(LNX_SYS_SHMDT,
			(unsigned long int[6]){
				(unsigned long int)addr,
				0,
				0,
				0,
				0,
				0
			},
			1
		);
	}

	if(id != -1){
		lnx_syscall(LNX_SYS_SHMCTL,
			(unsigned long int[6]){
				id,
				IPC_RMID,
				0x0,
				0,
				0,
				0
			},
			1
		);
	}
}
