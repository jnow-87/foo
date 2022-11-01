/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef INIT_SHELL_H
#define INIT_SHELL_H


#include <sys/types.h>
#include <sys/errno.h>
#include <sys/string.h>
#include <stdio.h>


/* macros */
#define ERROR(msg, ...)		shell_error(msg, ##__VA_ARGS__)
#define SHERROR(msg, ...)	ERROR("%s:%u " msg, shell_file, shell_line, ##__VA_ARGS__)


/* types */
typedef enum{
	SH_EIGN = 0x1,		// ignore shell errors
	SH_ONCE = 0x2,		// exit shell after first command
} shell_flags_t;


/* prototypes */
int shell_term(char const *prompt);
int shell_script(FILE *stream, char const *name, shell_flags_t flags);
int shell_error(char const *fmt, ...);


/* external variables */
extern char const *shell_file;
extern size_t shell_line;


#endif // INIT_SHELL_H
