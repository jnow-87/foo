/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LIB_TIME_H
#define LIB_TIME_H


#include <config/config.h>
#include <sys/compiler.h>
#include <sys/time.h>


/* prototypes */
int time(time_t *t);
uint32_t time_ms(void);


/* disabled-call macros */
#ifndef CONFIG_SC_TIME
# define time(t)	CALL_DISABLED(time, CONFIG_SC_TIME)
# define time_ms(t)	CALL_DISABLED(time_ms, CONFIG_SC_TIME)
#endif // CONFIG_SC_TIME


#endif // LIB_TIME_H
