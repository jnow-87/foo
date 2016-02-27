#ifndef ARCH_ATOMIC_H
#define ARCH_ATOMIC_H


#include <arch/arch.h>


/* macros */
#define cas(v, o, n)	(arch_common_call(cas, 0)(v, o, n))


#endif // ARCH_ATOMIC_H
