/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_STAT_H
#define KERNEL_STAT_H


#include <sys/compiler.h>


/* types */
typedef void (*stat_call_t)(void);


/* macros */
#define kernel_stat(call) \
	static stat_call_t stat_call_##call __linker_array(".kernel_stat") = call;


#endif // KERNEL_STAT_H
