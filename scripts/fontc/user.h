/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef FONTC_USER_H
#define FONTC_USER_H


#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>


/* macros */
#define ERROR(fmt, ...)	({ \
	fprintf(stderr, "error:%s:" fmt, (errno ? strerror(errno) : ""), ##__VA_ARGS__); \
	1; \
})


#endif // FONTC_USER_H
