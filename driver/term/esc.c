/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <driver/term.h>
#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/string.h>
#include <sys/math.h>
#include "term.h"


/* macros */
#define OPTVAL(term, default) (((term)->esc.nval == 0) ? default : ((term)->esc.val[0]))


/* local/static prototypes */
static int tab(term_t *term);
static int erase(term_t *term, term_erase_t type, uint16_t n);


/* global functions */
int esc_handle(term_t *term, char c){
	int r = 0;


	switch(esc_parse(&term->esc, c)){
	case ESC_INVAL:					r = -1; break;
	case ESC_PARTIAL:				return 0;

	case ESC_TAB:					r = tab(term); break;
	case ESC_BACKSPACE:				r = cursor_move(term, 0, -1, false); break;
	case ESC_CARRIAGE_RETURN:		r = cursor_set(term, term->cursor.line, 0); break;

	case ESC_NEWLINE:	// fall through
	case ESC_VERT_TAB:	// fall through
	case ESC_FORM_FEED:
		r = cursor_move(term, 1, 0, false);
		break;

	case ESC_CURSOR_MOVE_UP:		r = cursor_move(term, -OPTVAL(term, 1), 0, false); break;
	case ESC_CURSOR_MOVE_UP_HOME:	r = cursor_move(term, -OPTVAL(term, 1), -term->cursor.column, false); break;
	case ESC_CURSOR_MOVE_DOWN:		r = cursor_move(term, OPTVAL(term, 1), 0, false); break;
	case ESC_CURSOR_MOVE_DOWN_HOME:	r = cursor_move(term, OPTVAL(term, 1), -term->cursor.column, false); break;
	case ESC_CURSOR_MOVE_LEFT:		r = cursor_move(term, 0, -OPTVAL(term, 1), false); break;
	case ESC_CURSOR_MOVE_RIGHT:		r = cursor_move(term, 0, OPTVAL(term, 1), false); break;
	case ESC_CURSOR_MOVE_HOME:		r = cursor_set(term, term->esc.val[0], term->esc.val[1]); break;
	case ESC_CURSOR_SCROLL_DOWN:	r = cursor_move(term, 1, 0, true); break;
	case ESC_CURSOR_SCROLL_UP:		r = cursor_move(term, -1, 0, true); break;
	case ESC_CURSOR_SET_COLUMN:		r = cursor_set(term, term->cursor.line, OPTVAL(term, 0)); break;
	case ESC_CURSOR_RESTORE:		r = cursor_restore(term); break;
	case ESC_CURSOR_SAVE:			cursor_save(term); break;

	case ESC_ERASE_IN_DISPLAY:		r = erase(term, TE_MULTILINE, 0); break;
	case ESC_ERASE_IN_LINE:			r = erase(term, 0x0, 0); break;

	case ESC_SCROLL_SET:			term->cfg->lflags |= TLFL_SCROLL; r = 0; break;
	case ESC_WRAP_SET:				term->cfg->lflags |= TLFL_WRAP; r = 0; break;
	case ESC_WRAP_CLR:				term->cfg->lflags &= ~TLFL_WRAP; r = 0; break;

	case ESC_BELL:		// fall through
	case ESC_DELETE:	// fall through
	default:
		break;
	}

	esc_init(&term->esc);

	return r;
}


/* local functions */
static int tab(term_t *term){
	int r = 0;


	r |= erase(term, TE_TO_END, term->cfg->tabs);
	r |= cursor_move(term, 0, term->cfg->tabs, false);

	return r;
}

static int erase(term_t *term, term_erase_t type, uint16_t n){
	switch(term->esc.val[0]){
	case 0:		type |= TE_TO_END; break;
	case 1:		type |= TE_TO_START; break;
	case 2:		// fall through
	default:	type |= TE_TO_START | TE_TO_END; break;
	}

	return (term->itf->erase != 0x0) ? term->itf->erase(type, n, term->itf->hw) : 0;
}
