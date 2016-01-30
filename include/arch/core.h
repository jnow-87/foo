#ifndef ARCH_CORE_ID_H
#define ARCH_CORE_ID_H


#include <arch/arch.h>


/* macros */
#define PIR	arch_core_id()
#define core_id() arch_core_id()
#define arch_core_id() \
	arch_common_call(core_id, 0)()

#define core_halt() arch_core_halt()
#define arch_core_halt() \
	arch_common_call(core_halt, 0)()


#endif // ARCH_CORE_ID_H
