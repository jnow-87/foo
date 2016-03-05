#ifndef ARCH_INTERRUPT_H
#define ARCH_INTERRUPT_H


#include <arch/arch.h>
#include <sys/error.h>


/* macros */
#define int_enable(num)					(arch_kernel_call(int_enable, E_OK)(num))
#define int_disable(num)				(arch_kernel_call(int_disable, E_OK)(num))
#define int_enabled()					(arch_kernel_call(int_enabled, 0)())
#define int_hdlr_register(num, hdlr)	(arch_kernel_call(int_hdlr_register, E_OK)(num, hdlr))
#define int_hdlr_release(num)			(arch_kernel_call(int_hdlr_release, E_OK)(num))


#endif // ARCH_INTERRUPT_H
