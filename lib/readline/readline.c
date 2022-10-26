/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/types.h>
#include <sys/compiler.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/term.h>
#include <sys/escape.h>
#include <sys/string.h>
#include <lib/unistd.h>
#include <lib/stdio.h>
#include <lib/stdlib.h>
#include "history.h"


/* types */
typedef enum{
	ESC_NONE = 0,
	ESC_INVAL,
	ESC_UP,
	ESC_DOWN,
	ESC_LEFT,
	ESC_RIGHT,
} esc_t;


/* local/static prototypes */
static esc_t parse_esc(char const *s, size_t n);


/* global functions */
size_t readline_stdin(FILE *stream, char *line, size_t n){
	bool shadowed = false;
	size_t i = 0,
		   end = 0,
		   prev = 0;
	char *hst = 0x0;
	char c;
	int r;
	char shadow[n];
	esc_t esc;


	shadow[0] = 0;

	reset_errno();
	history_reset();

	while(end < n && i < n){
		/* get a character */
		r = read(fileno(stream), &c, 1);

		if(r < 0)
			goto err;

		if(r == 0)
			continue;

		/* special character handling */
		// end of line
		if(c == '\n'){
			write(fileno(stream), "\n", 1);
			line[end] = 0;

			if(end != 0)
				history_add(line);

			return end;
		}

		// backspace
		if(c == 127){
			if(i != end || i == 0)
				continue;

			write(fileno(stream), "\b \b", 3);

			end--;
			i = end;
			continue;
		}

		if(c == CTRL_C)
			exit(1);

		if(c == CTRL_D)
			exit(0);

		// skip windows line ending
		if(c == '\r')
			continue;

		// begin of escape sequence
		if(c == '\e'){
			prev = i;
			i = end;	// escape sequences are recorded to the end of the line
		}

		/* update line */
		line[i] = c;

		// print current character if not in an escape sequence
		if(line[end] != '\e'){
			if(i == end)
				end++;

			write(fileno(stream), &c, 1);
		}

		i++;

		/* parse escape sequences */
		if(line[end] == '\e'){
			esc = parse_esc(line + end, i - end);

			// sequence not yet complete
			if(esc == ESC_NONE)
				continue;

			// sequence complete
			// restore line index
			i = prev;

			// escape action
			switch(esc){
			case ESC_LEFT:
				if(i > 0){
					write(fileno(stream), line + end, 3);
					i--;
				}
				break;

			case ESC_RIGHT:
				if(i < end){
					write(fileno(stream), line + end, 3);
					i++;
				}
				break;

			case ESC_UP:
				hst = history_prev();
				break;

			case ESC_DOWN:
				hst = history_next();
				break;

			default:
				break;
			};

			// update the line content after moving through the command history
			if((esc == ESC_UP || esc == ESC_DOWN)){
				// save current line to the shadow buffer
				if(esc == ESC_UP && !shadowed){
					strncpy(shadow, line, end);
					shadow[end] = 0;
					shadowed = true;
				}

				// restore current line from shadow buffer
				if(esc == ESC_DOWN && hst == 0x0){
					strcpy(line, shadow);
					shadowed = false;
				}

				// update current line with history line
				if(hst)
					strcpy(line, hst);

				// update the terminal
				if(hst || !shadowed){
					end = strlen(line);
					i = end;

					fputs(RESTORE_POS CLEARLINE, stdout);	// restore pos to end of input prompt
					fwrite(line, end, stdout);
					fflush(stdout);
				}
			}

			// end current escape sequence handling
			line[end] = 0;
		}
	}


err:
	reset_errno();

	return 0;
}

size_t readline_regfile(FILE *stream, char *line, size_t n){
	size_t i = 0;
	int r;


	while(i < n){
		r = read(fileno(stream), line + i, 1);

		if(r < 0)
			goto err;

		if(r == 0 || line[i] == '\n'){
			line[i] = 0;
			return i;
		}

		if(line[i] == '\r')
			continue;

		i++;
	}


err:
	reset_errno();

	return 0;
}

void readline_history(void){
	history_dump();
}


/* local functions */
static esc_t parse_esc(char const *s, size_t n){
	static esc_t const codes[] = {
		ESC_UP, ESC_DOWN, ESC_RIGHT, ESC_LEFT
	};
	unsigned char i;


	if(n < 3 || *s != '\e' || *(s + 1) != '[')
		return ESC_NONE;

	i = *(s + 2) - 'A';

	if(i > sizeof_array(codes))
		return ESC_INVAL;

	return codes[i];
}
