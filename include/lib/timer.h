/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LIB_TIMER_H
#define LIB_TIMER_H


#include <config/config.h>
#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/signal.h>


/* prototypes */
int timer_register(signal_t sig, uint32_t period_us);
void timer_release(signal_t sig);


/* disabled-call macros */
#ifndef CONFIG_SC_TIME
# define timer_register(sig, period_us)	CALL_DISABLED(timer_register, CONFIG_SC_TIME)
# define timer_release(sig)				CALL_DISABLED(timer_release, CONFIG_SC_TIME)
#endif // CONFIG_SC_TIME


#endif // LIB_TIMER_H
