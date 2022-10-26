/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LIB_READLINE_H
#define LIB_READLINE_H


#include <sys/types.h>
#include <stdio.h>


/* prototypes */
size_t readline_stdin(FILE *stream, char *line, size_t n);
size_t readline_regfile(FILE *stream, char *line, size_t n);

void readline_history(void);


#endif // LIB_READLINE_H
