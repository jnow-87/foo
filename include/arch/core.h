#ifndef ARCH_CORE_ID_H
#define ARCH_CORE_ID_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define PIR				core_id()
#define core_id()		(arch_common_call(core_id, 0)())
#define core_sleep()	(arch_common_call(core_sleep, E_NOIMP)())
#define core_halt()		(arch_common_call(core_halt, E_NOIMP)())


#endif // ARCH_CORE_ID_H
