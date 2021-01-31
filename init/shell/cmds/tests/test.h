/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef CMD_TEST_H
#define CMD_TEST_H


#include <sys/escape.h>


/* macros */
#define test(_name, _exec, _descr) \
	static char const test_name_##_exec[]  __used = _name; \
	static char const test_descr_##_exec[]  __used = _descr; \
	static test_t test_##_exec __linker_array(".tests") = { \
		.name = test_name_##_exec, \
		.descr = test_descr_##_exec, \
		.exec = _exec, \
	}

#define ERROR(fmt, ...)	printf(FG_RED "error " RESET_ATTR fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)	printf(FG_YELLOW "warning " RESET_ATTR fmt, ##__VA_ARGS__)


/* types */
typedef struct{
	char const *name;
	char const *descr;
	int (*exec)(void);
} test_t;


#endif // CMD_TEST_H
