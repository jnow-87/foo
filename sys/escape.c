/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/compiler.h>
#include <sys/escape.h>
#include <sys/string.h>


/* local/static prototypes */
static esc_t parse_ctrl(esc_state_t *esc, char c);
static esc_t parse_esc(esc_state_t *esc, char c);
static esc_t parse_esc_square(esc_state_t *esc, char c);
static esc_t parse_number(esc_state_t *esc, char c);


/* global functions */
void esc_init(esc_state_t *esc){
	memset(esc, 0, sizeof(esc_state_t));
	esc->hdlr = parse_ctrl;
}

bool esc_active(esc_state_t *esc){
	return esc->hdlr != 0x0 && esc->hdlr != parse_ctrl;
}

esc_t esc_parse(esc_state_t *esc, char c){
	esc_t r;


	if(esc->hdlr == 0x0)
		esc_init(esc);

	r = esc->hdlr(esc, c);

	if(r != ESC_PARTIAL)
		esc->hdlr = 0x0;

	return r;
}


/* local functions */
static esc_t parse_ctrl(esc_state_t *esc, char c){
	switch(c){
	case '\033':
		esc->hdlr = parse_esc;
		return ESC_PARTIAL;

	case '\t':	return ESC_TAB;
	case '\b':	return ESC_BACKSPACE;
	case '\r':	return ESC_CARRIAGE_RETURN;
	case '\n':	return ESC_NEWLINE;
	case '\v':	return ESC_VERT_TAB;
	case '\f':	return ESC_FORM_FEED;
	case '\a':	return ESC_BELL;
	case 3:		return ESC_END_OF_TEXT;
	case 4:		return ESC_END_OF_TX;
	case 127:	return ESC_DELETE;
	default:	return ESC_INVAL;
	}
}

static esc_t parse_esc(esc_state_t *esc, char c){
	switch(c){
	case '[':
		esc->hdlr = parse_esc_square;
		return ESC_PARTIAL;

	case '7':	return ESC_CURSOR_SAVE;
	case '8':	return ESC_CURSOR_RESTORE;
	case 'D':	return ESC_CURSOR_SCROLL_DOWN;
	case 'M':	return ESC_CURSOR_SCROLL_UP;
	case 'H':	return ESC_TAB;
	default:	return ESC_INVAL;
	};
}

static esc_t parse_esc_square(esc_state_t *esc, char c){
	switch(c){
	/* cursor */
	case 's':	return ESC_CURSOR_SAVE;
	case 'u':	return ESC_CURSOR_RESTORE;
	case 'A':	return ESC_CURSOR_MOVE_UP;
	case 'B':	return ESC_CURSOR_MOVE_DOWN;
	case 'C':	return ESC_CURSOR_MOVE_RIGHT;
	case 'D':	return ESC_CURSOR_MOVE_LEFT;
	case 'E':	return ESC_CURSOR_MOVE_DOWN_HOME;
	case 'F':	return ESC_CURSOR_MOVE_UP_HOME;
	case 'G':	return ESC_CURSOR_SET_COLUMN;
	case 'f':	return ESC_CURSOR_MOVE_HOME;
	case 'H':	return ESC_CURSOR_MOVE_HOME;

	/* erase */
	case 'K':	return ESC_ERASE_IN_LINE;
	case 'J':	return ESC_ERASE_IN_DISPLAY;

	/* control */
	case 'r':	return ESC_SCROLL_SET;
	case 'h':	return (esc->val[0] == 7) ? ESC_WRAP_SET : ESC_INVAL;
	case 'l':	return (esc->val[0] == 7) ? ESC_WRAP_CLR : ESC_INVAL;

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
		esc->hdlr = parse_number;

		return parse_number(esc, c);

	/* default */
	default:
		return ESC_INVAL;
	};
}

static esc_t parse_number(esc_state_t *esc, char c){
	if(c == ';'){
		if(esc->nval == 0 || esc->nval >= sizeof_array(esc->val))
			return ESC_INVAL;

		esc->nval++;

		return ESC_PARTIAL;
	}

	if(esc->nval == 0)
		esc->nval++;

	if(c >= '0' && c <= '9'){
		esc->val[esc->nval - 1] *= 10;
		esc->val[esc->nval - 1] += c - '0';

		return ESC_PARTIAL;
	}

	esc->hdlr = parse_esc_square;

	return parse_esc_square(esc, c);
}
