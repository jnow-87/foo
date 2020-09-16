/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef INIT_HISTORY_H
#define INIT_HISTORY_H


/* prototypes */
void history_add(char *line);
void history_startover(void);
char *history_older(void);
char *history_newer(void);
void history_print(void);


#endif // INIT_HISTORY_H
