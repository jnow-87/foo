#ifndef ARCH_THREAD_H
#define ARCH_THREAD_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define thread_call(t)	(arch_kernel_call(thread_call, E_NOIMP)(t))
#define thread_kill(r)	(arch_kernel_call(thread_kill, E_NOIMP)(r))


#endif // ARCH_THREAD_H
