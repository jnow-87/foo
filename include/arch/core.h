#ifndef ARCH_CORE_ID_H
#define ARCH_CORE_ID_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define PIR				core_id()
#define core_id()		(arch_common_call(core_id, 0)())
#define core_sleep()	(arch_common_call(core_sleep, -E_NOIMP)())

#ifdef BUILD_KERNEL
#define core_panic()	(arch_kernel_call(core_panic, -E_NOIMP)())
#endif // BUILD_KERNEL


#endif // ARCH_CORE_ID_H
