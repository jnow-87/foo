/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/escape.h>
#include <sys/stat.h>
#include <lib/stdio.h>
#include <lib/stdlib.h>
#include <lib/unistd.h>
#include "line.h"
#include "history.h"


/* local/static prototypes */
static size_t read_chardev(int fd, char *line, size_t n);
static size_t read_file(int fd, char *line, size_t n);


/* global functions */
size_t readline(FILE *stream, char *line, size_t n){
	stat_t stat;


	if(fstat(fileno(stream), &stat) < 0)
		return 0;

	if(stat.type == FT_CHR)
		return read_chardev(fileno(stream), line, n);

	return read_file(fileno(stream), line, n);
}

void readline_history(void){
#ifdef CONFIG_READLINE_HISTORY
	history_dump();
#else
	printf("readline history is disabled\n");
#endif // CONFIG_READLINE_HISTORY
}


/* local functions */
static size_t read_chardev(int fd, char *line, size_t n){
	char c;
	char shadow[n];
	int r;
	esc_state_t esc;
	line_state_t lstate;


	esc_init(&esc);
	line_init(&lstate, line, shadow, n, fd);
	reset_errno();

#ifdef CONFIG_READLINE_HISTORY
	history_reset(&lstate);
#endif // CONFIG_READLINE_HISTORY

	while(!line_limit(&lstate)){
		r = read(fd, &c, 1);

		if(r == 0)
			continue;

		if(r < 0){
			printf("readline error: %s\n", strerror(errno));
			reset_errno();

			return 0;
		}

		switch(esc_parse(&esc, c)){
		case ESC_PARTIAL:			// fall through
		case ESC_FORM_FEED:			// fall through
		case ESC_CARRIAGE_RETURN:	// fall through
		case ESC_TAB:
			break;

		case ESC_NEWLINE:
			return line_complete(&lstate);

		case ESC_CTRL_C:			exit(1); break;
		case ESC_CTRL_D:			exit(0); break;

#ifdef CONFIG_READLINE_HISTORY
		case ESC_CURSOR_MOVE_UP:	history_cycle(&lstate, history_prev); break;
		case ESC_CURSOR_MOVE_DOWN:	history_cycle(&lstate, history_next); break;
#endif // CONFIG_READLINE_HISTORY

		case ESC_CURSOR_MOVE_LEFT:	line_cursor_left(&lstate); break;
		case ESC_CURSOR_MOVE_RIGHT:	line_cursor_right(&lstate); break;

		case ESC_DELETE:			line_char_del(&lstate); break;
		default:					line_char_add(&lstate, c); break;
		}
	}

	return 0;
}

static size_t read_file(int fd, char *line, size_t n){
	size_t i = 0;
	int r;


	while(i < n){
		r = read(fd, line + i, 1);

		if(r == 0)
			break;

		if(r < 0)
			goto err;

		if(line[i] == 0 || line[i] == '\n'){
			// don't return empty lines
			if(i == 0)
				continue;

			break;
		}

		if(line[i] == '\r')
			continue;

		i++;
	}

	line[i] = 0;

	return i;


err:
	reset_errno();

	return 0;
}
