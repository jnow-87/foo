/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_PANIC_H
#define KERNEL_PANIC_H


#include <kernel/thread.h>
#include <sys/stdarg.h>


/* macros */
#define kpanic(format, ...) \
	kpanic_ext(__FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)


/* prototypes */
void kpanic_ext(char const *file, char const *func, unsigned int line, char const *format, ...);


#endif // KERNEL_PANIC_H
