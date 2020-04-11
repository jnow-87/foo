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
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <shell/history.h>


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
static esc_t parse_esc(char const *s, size_t len);


/* global functions */
size_t readline_stdin(FILE *stream, char *line, size_t n){
	char c;
	size_t i,
		   end,
		   prev;
	int r;
	char *hst;
	char shadow[n];
	bool shadowed;
	esc_t esc;
	term_err_t terr;


	i = 0;
	end = 0;
	prev = 0;
	shadowed = false;
	shadow[0] = 0;
	hst = 0x0;

	errno = E_OK;
	history_startover();

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
				hst = history_older();
				break;

			case ESC_DOWN:
				hst = history_newer();
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
	if(errno == E_IO){
		ioctl(fileno(stream), IOCTL_STATUS, &terr, sizeof(term_err_t));
		fprintf(stderr, "readline I/O error: terminal error %#x\n", terr);
	}
	else if(errno)
		fprintf(stderr, "readline error on fd %d \"%s\"\n", fileno(stream), strerror(errno));

	errno = E_OK;

	return 0;
}

size_t readline_regfile(FILE *stream, char *line, size_t n){
	size_t i;
	int r;


	i = 0;

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
	if(errno){
		fprintf(stderr, "readline error on fd %d \"%s\"\n", fileno(stream), strerror(errno));
		errno = E_OK;
	}

	return 0;
}


/* local functions */
static esc_t parse_esc(char const *s, size_t len){
	static esc_t const codes[] = {
		ESC_UP, ESC_DOWN, ESC_RIGHT, ESC_LEFT
	};
	unsigned char i;


	if(len < 3 || *s != '\e' || *(s + 1) != '[')
		return ESC_NONE;

	i = *(s + 2) - 'A';

	if(i > sizeof_array(codes))
		return ESC_INVAL;

	return codes[i];
}
