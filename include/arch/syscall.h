/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_SYSCALL_H
#define ARCH_SYSCALL_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define sc(num, param)	(arch_common_call(sc, -E_NOIMP)(num, param, sizeof(*param)))


#endif // ARCH_SYSCALL_H
