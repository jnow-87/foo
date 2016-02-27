#ifndef ARCH_TIME_H
#define ARCH_TIME_H


#include <arch/arch.h>


/* macros */
#define timebase()				(arch_common_call(timebase, -1)())
#define timebase_to_time(tb)	(arch_common_call(timebase_to_time, -1)(tb))


#endif // ARCH_TIME_H
