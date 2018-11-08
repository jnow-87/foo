/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_TIME_H
#define SYS_TIME_H


#include <sys/types.h>


/* macros */
#define TIME_INITIALISER(){ \
		.s = 0, \
		.ms = 0, \
		.us = 0 \
	}


/* types */
typedef struct{
	uint32_t s;
	size_t ms,
		   us;
} time_t;


#endif // SYS_TIME_H
