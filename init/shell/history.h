/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef INIT_HISTORY_H
#define INIT_HISTORY_H


#include <config/config.h>


/* macros */
#ifndef CONFIG_INIT_HISTORY

#define history_add(line)	{}
#define history_startover()	{}
#define history_older()		((void*)0x0)
#define history_newer()		((void*)0x0)

#endif // CONFIG_INIT_HISTORY


/* prototypes */
#ifdef CONFIG_INIT_HISTORY

void history_add(char *line);
void history_startover(void);
char *history_older(void);
char *history_newer(void);
void history_print(void);

#endif // CONFIG_INIT_HISTORY


#endif // INIT_HISTORY_H
