#ifndef ARCH_IO_H
#define ARCH_IO_H


#include <arch/arch.h>
#include <sys/error.h>


/* macros */
#define putchar(c)	(arch_kernel_call(putchar, E_NOIMP)(c))
#define puts(s)		(arch_kernel_call(puts, E_NOIMP)(s))


#endif // ARCH_IO_H
