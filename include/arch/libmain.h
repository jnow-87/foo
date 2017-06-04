#ifndef ARCH_LIBMAIN_H
#define ARCH_LIBMAIN_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define libmain(argc, argv)	(arch_common_call(libmain, 1)(argc, argv))


#endif // ARCH_LIBMAIN_H
