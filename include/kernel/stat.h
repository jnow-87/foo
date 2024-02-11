/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_STAT_H
#define KERNEL_STAT_H


#include <config/config.h>
#include <sys/compiler.h>


/* types */
typedef void (*stat_call_t)(void);


/* macros */
#ifdef CONFIG_KERNEL_STAT
# define kernel_stat(call) \
	static stat_call_t stat_call_##call __linker_array(".kernel_stat") = call
#else
# define kstat()

# define kernel_stat(call) \
	static void call(void) __unused __discard
#endif // CONFIG_KERNEL_STAT


/* prototypes */
#ifdef CONFIG_KERNEL_STAT
void kstat(void);
#endif // CONFIG_KERNEL_STAT


#endif // KERNEL_STAT_H
