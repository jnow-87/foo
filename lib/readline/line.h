/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LIB_READLINE_LINE_H
#define LIB_READLINE_LINE_H


#include <sys/types.h>
#include <sys/string.h>
#include <lib/unistd.h>


/* macros */
#define WRITE(fd, s)	write(fd, s, strlen(s))


/* types */
typedef struct{
	char *line,
		 *shadow_line;

	bool shadowed;
	size_t capa,
		   pos,
		   last;

	int fd;
} line_state_t;


/* prototypes */
void line_init(line_state_t *state, char *line, char *shadow, size_t capa, int fd);
size_t line_complete(line_state_t *state);

bool line_limit(line_state_t *state);

void line_char_add(line_state_t *state, char c);
void line_char_del(line_state_t *state);

void line_cursor_left(line_state_t *state);
void line_cursor_right(line_state_t *state);


#endif // LIB_READLINE_LINE_H
