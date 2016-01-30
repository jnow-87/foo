#ifndef ARCH_IO_H
#define ARCH_IO_H


#include <arch/arch.h>


/* macro */
#define putchar(c) arch_putchar(c)
#define arch_putchar(c) \
	arch_kernel_call(putchar, -1)(c)

#define getchar() arch_getchar()
#define arch_getchar() \
	arch_kernel_call(getchar, -1)()


#endif // ARCH_IO_H
