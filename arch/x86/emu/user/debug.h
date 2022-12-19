/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86EMU_DEBUG_H
#define X86EMU_DEBUG_H


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/escape.h>
#include <user/opts.h>


/* macros */
#define DEBUG(lvl, fmt, ...) ((opts.verbosity < lvl + 2) ? (void)0 : \
	fprintf(stderr, \
		FG_YELLOW "[" PROGNAME "] " FG_VIOLETT " %25.25s:%-5u " FG_YELLOW "%-20.20s " RESET_ATTR fmt "\r", \
		__FILE__, \
		__LINE__, \
		__FUNCTION__, \
		##__VA_ARGS__ \
	) \
)

#define ERROR(fmt, ...) \
	fprintf(stderr, \
		FG_YELLOW "[" PROGNAME "] " FG_RED "error" FG_VIOLETT " %20.20s:%-5u " FG_YELLOW "%-20.20s "RESET_ATTR fmt "\r", \
		__FILE__, \
		__LINE__, \
		__FUNCTION__, \
		##__VA_ARGS__ \
	)

#define EEXIT(msg, ...){ \
	ERROR(msg, ##__VA_ARGS__); \
	exit(1); \
}


#endif // X86EMU_DEBUG_H
