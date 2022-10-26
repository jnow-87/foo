/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LIB_READLINE_HISTORY_H
#define LIB_READLINE_HISTORY_H


/* prototypes */
void history_reset(void);
void history_dump(void);

void history_add(char *line);

char *history_prev(void);
char *history_next(void);


#endif // LIB_READLINE_HISTORY_H
