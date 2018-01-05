#ifndef ARCH_CORE_ID_H
#define ARCH_CORE_ID_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define PIR				core_id()
#define core_id()		(arch_kernel_call(core_id, 0)())
#define core_sleep()	(arch_kernel_call(core_sleep, -E_NOIMP)())
#define core_panic(ctx)	(arch_kernel_call(core_panic, -E_NOIMP)(ctx))


#endif // ARCH_CORE_ID_H
