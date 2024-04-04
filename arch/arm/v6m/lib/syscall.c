/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/errno.h>
#include <sys/syscall.h>
#include <sys/types.h>


/* global functions */
int av6m_sc(sc_num_t num, void *param, size_t psize){
	sc_t volatile sc;


	/* prepare paramter */
	sc.num = num;
	sc.param = param;
	sc.size = psize;
	sc.errnum = E_UNKNOWN;

	/* trigger syscall */
	asm volatile(
		"mov	r0, %[sc]\n"
		"svc	0x0\n"
		:
		: [sc] "r" (&sc)
		: "r0"
	);

	if(sc.errnum)
		return_errno(sc.errnum);

	return 0;
}
