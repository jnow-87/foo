#ifndef ARCH_IPI_H
#define ARCH_IPI_H


#include <arch/arch.h>


/* macros */
#define ipi_sleep()					arch_common_call(ipi_sleep, 0)()
#define ipi_wake(num, core, bcast)	arch_common_call(ipi_wake, 0)(num, core, bcast)


#endif // ARCH_IPI_H
