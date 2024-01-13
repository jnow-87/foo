/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_ATOMIC_H
#define ARCH_ATOMIC_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define atomic(op, param)	(arch_common_call(atomic, -E_NOIMP)(op, param))


#endif // ARCH_ATOMIC_H
