#ifndef KERNEL_STAT_H
#define KERNEL_STAT_H


#include <sys/compiler.h>


/* types */
typedef void (*stat_call_t)(void);


/* macros */
#define kernel_stat(call)	static stat_call_t stat_call_##call __section(".kernel_stat") __used = call;


#endif // KERNEL_STAT_H
