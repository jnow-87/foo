/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <sys/types.h>
#include <sys/limits.h>
#include <sys/list.h>
#include <sys/string.h>
#include <sys/escape.h>
#include <lib/stdio.h>
#include <lib/stdlib.h>
#include "line.h"
#include "history.h"


/* types */
typedef struct history_t{
	struct history_t *prev,
					 *next;

	char line[LINE_MAX];
} history_t;


/* static variables */
static history_t *history = 0x0,
				 *next = 0x0;
static int direction = 0;


/* global functions */
void history_reset(line_state_t *state){
	next = history;
	direction = 0;

	WRITE(state->fd, STORE_POS);
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

void history_cycle(line_state_t *state, char * (*cycle)(void)){
	char *hst;


	hst = cycle();

	// restore line from shadow buffer
	if(cycle == history_next && hst == 0x0){
		strcpy(state->line, state->shadow_line);
		state->shadowed = false;
	}

	// save line to the shadow buffer
	if(cycle == history_prev && !state->shadowed){
		strncpy(state->shadow_line, state->line, state->last);
		state->shadow_line[state->last] = 0;
		state->shadowed = true;
	}

	// update line with history buffer
	if(hst != 0x0)
		strcpy(state->line, hst);

	// update the terminal
	if(hst != 0x0 || !state->shadowed){
		state->last = strlen(state->line);
		state->pos = state->last;

		WRITE(state->fd, RESTORE_POS CLEARLINE);
		write(state->fd, state->line, state->last);
	}
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
