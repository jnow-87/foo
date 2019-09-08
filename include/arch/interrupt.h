/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_INTERRUPT_H
#define ARCH_INTERRUPT_H


#include <sys/types.h>


/* types */
typedef uint8_t int_num_t;
typedef void (*int_hdlr_t)(int_num_t num, void *data);


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define int_register(num, hdlr, data)	(arch_kernel_call(int_register, -E_NOIMP)(num, hdlr, data))
#define int_release(num)				(arch_kernel_call(int_release, -E_NOIMP)(num))

#define int_call(num)					(arch_kernel_call(int_call, -E_NOIMP)(num))

#define int_enable(mask)				(arch_kernel_call(int_enable, INT_NONE)(mask))
#define int_enabled()					(arch_kernel_call(int_enabled, INT_NONE)())

#define int_ipi(core, bcast)			(arch_kernel_call(int_ipi, -E_NOIMP)(core, bcast))


#endif // ARCH_INTERRUPT_H
