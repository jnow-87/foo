/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <sys/list.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shell/shell.h>
#include <shell/history.h>
#include <shell/cmd.h>


/* types */
typedef struct el_t{
	struct el_t *prev,
				*next;

	char line[CONFIG_LINE_MAX];
} el_t;

/* local/static prototypes */
static int print(int argc, char **argv);


/* static variables */
static el_t *history = 0x0,
			*rd = 0x0;
static int dir = 0;


/* global functions */
void history_add(char *line){
	el_t *e;


	if(history && strcmp(history->line, line) == 0)
		return;

	e = malloc(sizeof(el_t));

	if(e){
		strcpy(e->line, line);
		list_add_head(history, e);
		rd = e;
	}
	else
		SHELL_ERROR("cannot add to history, %s\n", strerror(errno));
}

void history_startover(void){
	rd = history;
	dir = 0;
}

char *history_older(void){
	char *r;


	if(dir < 0 && rd != 0x0)
		rd = rd->next;

	dir = 1;

	if(rd == 0x0)
		return 0x0;

	r = rd->line;
	rd = rd->next;

	return r;
}

char *history_newer(void){
	if(dir > 0)
		rd = (rd == 0x0) ? list_last(history) : rd->prev;

	if(rd == history){
		dir = 0;
		return 0x0;
	}

	rd = (rd == 0x0) ? list_last(history) : rd->prev;
	dir = -1;

	return (rd == 0x0) ? 0x0 : rd->line;
}


/* local functions */
static int print(int argc, char **argv){
	size_t i;


	i = 1;
	rd = list_last(history);

	do{
		printf("%u: %s\n", i++, rd->line);
		rd = rd->prev;
	}while(rd != list_last(history));

	return 0;
}

command("history", print);
