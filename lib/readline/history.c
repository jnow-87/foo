/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <sys/list.h>
#include <sys/string.h>
#include <lib/stdio.h>
#include <lib/stdlib.h>
#include "history.h"


/* types */
typedef struct history_t{
	struct history_t *prev,
					 *next;

	char line[CONFIG_LINE_MAX];
} history_t;


/* static variables */
static history_t *history = 0x0,
				 *next = 0x0;
static int direction = 0;


/* global functions */
void history_reset(void){
	next = history;
	direction = 0;
}

void history_dump(void){
	size_t i = 1;


	next = list_last(history);

	do{
		printf("%u: %s\n", i++, next->line);
		next = next->prev;
	}while(next != list_last(history));
}

void history_add(char *line){
	history_t *e;


	if(history && strcmp(history->line, line) == 0)
		return;

	e = malloc(sizeof(history_t));

	if(e == 0x0){
		e = list_last(history);

		if(e == 0x0)
			return;

		list_rm(history, e);
	}

	strcpy(e->line, line);
	list_add_head(history, e);
	next = e;
}

char *history_prev(void){
	char *line;


	if(direction < 0 && next != 0x0)
		next = next->next;

	direction = 1;

	if(next == 0x0)
		return 0x0;

	line = next->line;
	next = next->next;

	return line;
}

char *history_next(void){
	if(direction > 0)
		next = (next == 0x0) ? list_last(history) : next->prev;

	if(next == history){
		direction = 0;

		return 0x0;
	}

	next = (next == 0x0) ? list_last(history) : next->prev;
	direction = -1;

	return (next == 0x0) ? 0x0 : next->line;
}
