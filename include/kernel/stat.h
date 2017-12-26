#ifndef KERNEL_STAT_H
#define KERNEL_STAT_H


#include <config/config.h>
#include <sys/compiler.h>


/* types */
typedef void (*stat_call_t)(void);


/* macros */
#define kernel_stat(call)	static stat_call_t stat_call_##call __section(".kernel_stat") __used = call;


/* prototypes */
#ifdef CONFIG_KERNEL_STAT
void kstat();
#else
#define kstat()
#endif // CONFIG_KERNEL_STAT


#endif // KERNEL_STAT_H
