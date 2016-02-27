#ifndef ARCH_IO_H
#define ARCH_IO_H


#include <arch/arch.h>


/* macros */
#define putchar(c)	(arch_kernel_call(putchar, -1)(c))
#define puts(s)		(arch_kernel_call(puts, -1)(s))


#endif // ARCH_IO_H
