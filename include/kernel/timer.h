/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H


#include <config/config.h>


/* macros */
#ifndef CONFIG_SC_TIME

#define ktimer_tick()	{}

#endif // CONFIG_SC_TIME


/* prototypes */
#ifdef CONFIG_SC_TIME

void ktimer_tick(void);

#endif // CONFIG_SC_TIME


#endif // KERNEL_TIMER_H
