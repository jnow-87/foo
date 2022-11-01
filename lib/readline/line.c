/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#include <config/config.h>
#include <sys/stream.h>
#include <lib/unistd.h>
#include "line.h"
#include "history.h"


/* global functions */
void line_init(line_state_t *state, char *line, char *shadow, size_t capa, int fd){
	state->line = line;
	state->shadow_line = shadow;
	state->shadowed = false;
	state->capa = capa;
	state->pos = 0;
	state->last = 0;
	state->fd = fd;

	shadow[0] = 0;
}

size_t line_complete(line_state_t *state){
	WRITE(state->fd, "\n");
	state->line[state->last] = 0;

#ifdef CONFIG_READLINE_HISTORY
	if(state->last != 0)
		history_add(state->line);
#endif // CONFIG_READLINE_HISTORY

	return state->last;
}

bool line_limit(line_state_t *state){
	// +8: buffer for cursor movement escape sequences
	return state->last + 8 >= state->capa;
}

void line_char_add(line_state_t *state, char c){
	size_t len = state->last - state->pos;


	/* update line */
	memcpy(state->line + state->pos + 1, state->line + state->pos, len);
	state->line[state->pos] = c;

	/* update terminal */
	state->line[state->last + 1] = 0;

	if(len > 0)
		snprintf(state->line + state->last + 1, 8, "\033[%uD", len);

	write(state->fd, state->line + state->pos, len + 1 + strlen(state->line + state->last + 1));

	state->last++;
	state->pos++;
}

void line_char_del(line_state_t *state){
	size_t len = state->last - state->pos;


	if(state->pos == 0)
		return;

	/* update terminal */
	state->line[state->pos - 1] = '\b';
	state->line[state->last] = ' ';
	snprintf(state->line + state->last + 1, 8, "\033[%uD", len + 1);
	write(state->fd, state->line + state->pos - 1, len + 2 + strlen(state->line + state->last + 1));

	/* update line */
	memcpy(state->line + state->pos - 1, state->line + state->pos, len);

	state->pos--;
	state->last--;
}

void line_cursor_left(line_state_t *state){
	if(state->pos == 0)
		return;

	WRITE(state->fd, "\033[D");
	state->pos--;
}

void line_cursor_right(line_state_t *state){
	if(state->pos >= state->last)
		return;

	WRITE(state->fd, "\033[C");
	state->pos++;
}
