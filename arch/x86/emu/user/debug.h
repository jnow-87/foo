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
		FG("[" PROGNAME "] ", YELLOW) FG(" %25.25s:%-5u ", PURPLE) FG("%-20.20s ", YELLOW) fmt "\r", \
		__FILE__, \
		__LINE__, \
		__FUNCTION__, \
		##__VA_ARGS__ \
	) \
)

#define ERROR(fmt, ...) \
	fprintf(stderr, \
		FG("[" PROGNAME "] ", YELLOW) FG("error", RED) FG(" %20.20s:%-5u ", PURPLE) FG("%-20.20s ", YELLOW) fmt "\r", \
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
