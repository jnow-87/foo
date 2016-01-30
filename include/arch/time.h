#ifndef ARCH_TIME_H
#define ARCH_TIME_H


#include <arch/arch.h>


/* macros */
#define timebase() arch_timebase()
#define arch_timebase() \
	arch_common_call(timebase, 0)()

#define timebase_to_time(tb) arch_timebase_to_time(tb)
#define arch_timebase_to_time(tb) \
	arch_common_call(timebase_to_time, 0)(tb)


#endif // ARCH_TIME_H
