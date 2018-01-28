#ifndef ARCH_INTERRUPT_H
#define ARCH_INTERRUPT_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define int_enable(mask)				(arch_kernel_call(int_enable, INT_NONE)(mask))
#define int_enabled()					(arch_kernel_call(int_enabled, INT_NONE)())


#endif // ARCH_INTERRUPT_H
