#ifndef KERNEL_STAT_H
#define KERNEL_STAT_H


#include <config/config.h>


/* prototypes */
#ifdef CONFIG_KERNEL_STAT
void kstat();
#else
#define kstat()
#endif // CONFIG_KERNEL_STAT


#endif // KERNEL_STAT_H
