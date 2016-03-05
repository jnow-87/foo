#ifndef ARCH_ATOMIC_H
#define ARCH_ATOMIC_H


#include <arch/arch.h>
#include <sys/error.h>


/* macros */
#define cas(v, o, n)	(arch_common_call(cas, E_OK)(v, o, n))


#endif // ARCH_ATOMIC_H
