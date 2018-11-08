/*
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LIB_SCHED_H
#define LIB_SCHED_H


#include <config/config.h>
#include <sys/compiler.h>


/* prototypes */
void sched_yield(void);


/* disabled-call macros */
#ifndef CONFIG_SC_SCHED

#define sched_yield()	CALL_DISABLED(sched_yield, CONFIG_SC_SCHED)

#endif // CONFIG_SC_SCHED


#endif // LIB_SCHED_H
