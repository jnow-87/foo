#ifndef ARCH_IPI_H
#define ARCH_IPI_H


#include <arch/arch.h>


/* macro */
#define ipi_sleep() arch_ipi_sleep()
#define arch_ipi_sleep() \
	arch_common_call(ipi_sleep, 0)()

#define ipi_wake(num, core, bcast) arch_ipi_wake(num, core, bcast)
#define arch_ipi_wake(num, core, bcast) \
	arch_common_call(ipi_wake, 0)(num, core, bcast)


#endif // ARCH_IPI_H
