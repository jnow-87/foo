#ifndef ARCH_LIBMAIN_H
#define ARCH_LIBMAIN_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define lib_init()				(arch_common_call(lib_init, E_NOIMP)())
#define lib_main(argc, argv)	(arch_common_call(lib_main, E_NOIMP)(argc, argv))


#endif // ARCH_LIBMAIN_H
