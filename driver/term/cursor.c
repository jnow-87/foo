/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <driver/term.h>
#include <sys/types.h>
#include <sys/math.h>
#include "term.h"


/* global functions */
int cursor_set(term_t *term, uint16_t line, uint16_t column){
	if(term->itf->cursor == 0x0)
		return 0;

	term->cursor.line = MIN(line, term->cfg->lines - 1);
	term->cursor.column = MIN(column, term->cfg->columns - 1);

	return term->itf->cursor(term->cursor.line, term->cursor.column, 0, term->itf->hw);
}

int cursor_move(term_t *term, int16_t lines, int16_t columns, bool force_scroll){
	if(columns < 0)
		columns = -MIN((uint16_t)-columns, term->cursor.column);

	if(term->cursor.column + columns >= term->cfg->columns){
		if(WRAP(term)){
			columns = -(term->cfg->columns - columns);
			lines++;
		}
		else
			columns = term->cfg->columns - term->cursor.column - 1;
	}

	if(lines < 0 && (uint16_t)-lines > term->cursor.line){
		if(term->itf->scroll != 0x0 && (SCROLL(term) || force_scroll)){
			lines += term->cursor.line;

			if(term->itf->scroll(lines, term->itf->hw) != 0)
				return -errno;
		}

		lines = -term->cursor.line;
	}
	else if(term->cursor.line + lines >= term->cfg->lines){
		if(term->itf->scroll != 0x0 && (SCROLL(term) || force_scroll)){
			lines = term->cursor.line + lines - term->cfg->lines + 1;

			if(term->itf->scroll(lines, term->itf->hw) != 0)
				return -errno;
		}

		lines = term->cfg->lines - term->cursor.line - 1;
	}

	return cursor_set(term, term->cursor.line + lines, term->cursor.column + columns);
}

void cursor_save(term_t *term){
	term->cursor.save_lime = term->cursor.line;
	term->cursor.save_column = term->cursor.column;
}

int cursor_restore(term_t *term){
	return cursor_set(term, term->cursor.save_lime, term->cursor.save_column);
}

int cursor_show(term_t *term, bool show){
	if(show == term->cursor.visible)
		return 0;

	if(term->itf->cursor(term->cursor.line, term->cursor.column, true, term->itf->hw) != 0)
		return -errno;

	term->cursor.visible = show;

	return 0;
}
