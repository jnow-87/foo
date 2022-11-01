/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LIB_READLINE_HISTORY_H
#define LIB_READLINE_HISTORY_H


#include "line.h"


/* prototypes */
void history_reset(line_state_t *state);
void history_dump(void);

void history_add(char *line);
void history_cycle(line_state_t *state, char * (*cycle)(void));

char *history_prev(void);
char *history_next(void);


#endif // LIB_READLINE_HISTORY_H
