/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_INTERRUPT_H
#define ARCH_INTERRUPT_H


#include <arch/arch.h>
#include <sys/errno.h>
#include <sys/types.h>


/* macros */
#define int_enable(mask)			(arch_kernel_call(int_enable, false)(mask))
#define int_enabled()				(arch_kernel_call(int_enabled, false)())

#define ipi_int(core, bcast, msg)	(arch_kernel_call(ipi_int, -E_NOIMP)(core, bcast, msg))
#define ipi_arg()					(arch_kernel_call(ipi_arg, 0x0)())


#endif // ARCH_INTERRUPT_H
