/*
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_TIME_H
#define ARCH_TIME_H


#include <arch/arch.h>


/* macros */
#define timebase()				(arch_common_call(timebase, 0)())
#define timebase_to_time(tb)	(arch_common_call(timebase_to_time, 0)(tb))


#endif // ARCH_TIME_H
