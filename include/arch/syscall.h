#ifndef ARCH_SYSCALL_H
#define ARCH_SYSCALL_H


#include <arch/arch.h>


/* macros */
#define syscall(num, param)	(arch_common_call(syscall, -1)(num, param, sizeof(*param)))


#endif // ARCH_SYSCALL_H
