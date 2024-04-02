/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/rp2040.h>
#include <arch/arm/v6m.h>
#include <arch/types.h>
#include <sys/register.h>
#include <sys/types.h>


/* macros */
#define SPINLOCK(id)	MREG(SIO_BASE + 0x100 + id * 4)

// use separate hardware spinlocks for kernel and user space since user space
// atomic sections are not protected through disabled interrupts, hence using
// the same spinlock could deadlock the system
#ifdef BUILD_KERNEL
# define SPINLOCK_ID	0
#else
# define SPINLOCK_ID	1
#endif // BUILD_KERNEL


/* global functions */
int rp2040_atomic(atomic_t op, void *param){
#ifdef BUILD_KERNEL
	bool int_en;
#endif // BUILD_KERNEL
	int r;


#ifdef BUILD_KERNEL
	// interrupts cannot be controlled in user space (CONTROL[nPRIV] == 1)
	// to make this obvious, make it conditional here, even though the processor
	// would simply ignore those instructions without causing a fault
	int_en = av6m_int_enable(false);
#endif // BUILD_KERNEL

	while(SPINLOCK(SPINLOCK_ID) == 0);

	r = op(param);

	SPINLOCK(SPINLOCK_ID) = 1;

#ifdef BUILD_KERNEL
	av6m_int_enable(int_en);
#endif // BUILD_KERNEL

	return r;
}
