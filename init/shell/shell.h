/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef INIT_SHELL_H
#define INIT_SHELL_H


#include <sys/types.h>
#include <stdio.h>


/* macros */
#define SHELL_ERROR(msg, ...) \
	fprintf(stderr, "%s:%u " msg, shell_file, shell_line, ##__VA_ARGS__);


/* external variables */
extern char shell_file[];
extern size_t shell_line;


/* prototypes */
int shell(char const *prompt, FILE *stream);


#endif // INIT_SHELL_H
