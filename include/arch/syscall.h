#ifndef ARCH_SYSCALL_H
#define ARCH_SYSCALL_H


#include <arch/arch.h>


/* macros */
#define arch_syscall(num, param, size) \
	arch_common_call(syscall, -1)(num, param, size)


#endif // ARCH_SYSCALL_H
