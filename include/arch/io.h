/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_IO_H
#define ARCH_IO_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define putchar(c)	(arch_kernel_call(putchar, !c)(c))
#define puts(s)		(arch_kernel_call(puts, 0)(s))


#endif // ARCH_IO_H
