#ifndef ARCH_THREAD_H
#define ARCH_THREAD_H


#include <arch/arch.h>


/* macros */
#define thread_call(t)	arch_kernel_call(thread_call, 0)(t)
#define thread_kill(r)	arch_kernel_call(thread_kill, 0)(r)


#endif // ARCH_THREAD_H
