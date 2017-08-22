#ifndef ARCH_IPI_H
#define ARCH_IPI_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define ipi_sleep()					(arch_common_call(ipi_sleep, -E_NOIMP)())
#define ipi_wake(num, core, bcast)	(arch_common_call(ipi_wake, -E_NOIMP)(num, core, bcast))


#endif // ARCH_IPI_H
