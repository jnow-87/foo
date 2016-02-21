#ifndef ARCH_IO_H
#define ARCH_IO_H


#include <arch/arch.h>


/* macros */
#define putchar(c)	arch_kernel_call(putchar, -1)(c)
#define getchar()	arch_kernel_call(getchar, -1)()


#endif // ARCH_IO_H
