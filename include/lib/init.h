/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LIB_INIT_H
#define LIB_INIT_H


#include <sys/compiler.h>


/* types */
typedef int (*lib_init_call_t)(void);


/* macros */
#define lib_init(stage, call)	static lib_init_call_t init_call_##call __section(".lib_init_stage"#stage) __used = call


#endif // LIB_INIT_H
