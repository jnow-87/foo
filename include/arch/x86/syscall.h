/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86_SYSCALL_H
#define X86_SYSCALL_H


#include <sys/types.h>
#include <sys/syscall.h>


/* prototypes */
int x86_sc(sc_num_t num, void *param, size_t psize);


#endif // X86_SYSCALL_H
