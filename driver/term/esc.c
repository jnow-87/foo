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
static void *parse_ctrl(term_t *term, char c);
static void *parse_esc(term_t *term, char c);
static void *parse_esc_square(term_t *term, char c);
static void *parser_number(term_t *term, char c);

static void *reset(term_t *term);

static int tab(term_t *term);
static int erase(term_t *term, term_erase_t type, uint16_t n);


/* global functions */
int term_esc_handle(term_t *term, char c){
	term->esc.hdlr = term->esc.hdlr(term, c);

	if(term->esc.hdlr != 0x0)
		return 0;

	(void)reset(term);

	return -errno;
}

void term_esc_reset(term_t *term){
	(void)reset(term);
}

bool term_esc_active(term_t *term){
	return term->esc.hdlr != parse_ctrl;
}


/* local functions */
static void *parse_ctrl(term_t *term, char c){
	int r;


	r = 0;

	switch(c){
	case '\033':	return parse_esc;

	case '\t':		r = tab(term); break;
	case '\b':		r = term_cursor_move(term, 0, -1, false); break;
	case '\r':		r = term_cursor_set(term, term->cursor.line, 0); break;

	case '\n':		// fall through
	case '\v':		// fall through
	case '\f':
		r = term_cursor_move(term, 1, 0, false);
		break;

	case '\a':		// fall through (bell)
	case '\177':	// fall through (delete = noop)
	default:
		break;
	}

	return (r == 0) ? reset(term) : 0x0;
}

static void *parse_esc(term_t *term, char c){
	int r;


	r = 0;

	switch(c){
	case '[':	return parse_esc_square;

	case '7':	term_cursor_save(term); break;
	case '8':	r = term_cursor_restore(term); break;
	case 'D':	r = term_cursor_move(term, 1, 0, true); break;
	case 'M':	r = term_cursor_move(term, -1, 0, true); break;
	case 'H':	r = tab(term); break;

	default:
		break;
	};

	return (r == 0) ? reset(term) : 0x0;
}

static void *parse_esc_square(term_t *term, char c){
	int r;


	r = 0;

	switch(c){
	/* cursor */
	case 's':	term_cursor_save(term); break;
	case 'u':	r = term_cursor_restore(term); break;
	case 'A':	r = term_cursor_move(term, -OPTVAL(term, 1), 0, false); break;
	case 'B':	r = term_cursor_move(term, OPTVAL(term, 1), 0, false); break;
	case 'C':	r = term_cursor_move(term, 0, OPTVAL(term, 1), false); break;
	case 'D':	r = term_cursor_move(term, 0, -OPTVAL(term, 1), false); break;
	case 'E':	r = term_cursor_move(term, OPTVAL(term, 1), -term->cursor.column, false); break;
	case 'F':	r = term_cursor_move(term, -OPTVAL(term, 1), -term->cursor.column, false); break;
	case 'G':	r = term_cursor_set(term, term->cursor.line, OPTVAL(term, 0)); break;

	case 'f':	// fall through
	case 'H':
		r = term_cursor_set(term, term->esc.val[0], term->esc.val[1]);
		break;

	/* erase */
	case 'K':	r = erase(term, 0x0, 0); break;
	case 'J':	r = erase(term, TE_MULTILINE, 0); break;

	/* control */
	case 'r':	term->cfg->lflags |= TLFL_SCROLL; break;

	case 'h':
		if(term->esc.val[0] == 7)
			term->cfg->lflags |= TLFL_WRAP;
		break;

	case 'l':
		if(term->esc.val[0] == 7)
			term->cfg->lflags &= ~TLFL_WRAP;
		break;

	/* number */
	case '0':	// fall through
	case '1':	// fall through
	case '2':	// fall through
	case '3':	// fall through
	case '4':	// fall through
	case '5':	// fall through
	case '6':	// fall through
	case '7':	// fall through
	case '8':	// fall through
	case '9':	// fall through
	case ';':
		return parser_number(term, c);

	/* default */
	default:
		break;
	};

	return (r == 0) ? reset(term) : 0x0;
}

static void *parser_number(term_t *term, char c){
	term_esc_t *esc;


	esc = &term->esc;

	if(c == ';'){
		if(esc->nval == 0 || esc->nval > sizeof_array(esc->val))
			return reset(term);

		term->esc.nval++;

		return parser_number;
	}

	if(esc->nval == 0)
		esc->nval++;

	if(c >= '0' && c <= '9'){
		esc->val[esc->nval - 1] *= 10;
		esc->val[esc->nval - 1] += c - '0';

		return parser_number;
	}

	return parse_esc_square(term, c);
}

static void *reset(term_t *term){
	memset(&term->esc, 0, sizeof(term_esc_t));
	term->esc.hdlr = parse_ctrl;

	return parse_ctrl;
}

static int tab(term_t *term){
	int r;


	r = 0;
	r |= erase(term, TE_TO_END, term->cfg->tabs);
	r |= term_cursor_move(term, 0, term->cfg->tabs, false);

	return r;
}

static int erase(term_t *term, term_erase_t type, uint16_t n){
	switch(term->esc.val[0]){
	case 0:		type |= TE_TO_END; break;
	case 1:		type |= TE_TO_START; break;
	case 2:		// fall through
	default:	type |= TE_TO_START | TE_TO_END; break;
	}

	return (term->hw->erase != 0x0) ? term->hw->erase(type, n, term->hw->data) : 0;
}
