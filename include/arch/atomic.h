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
#include <sys/compiler.h>


/* macros */
#define cas(v, o, n)		(arch_common_call(cas, 1)(v, o, n))
#define atomic_inc(v, inc)	(arch_common_call(atomic_inc, -E_NOIMP)(v, inc))


#endif // ARCH_ATOMIC_H
