/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_ESCAPE_H
#define SYS_ESCAPE_H


#include <sys/types.h>


/* macros */
// named ascii codes
#define END_OF_TEXT	3
#define END_OF_TX	4

#define CTRL_C		END_OF_TEXT
#define CTRL_D		END_OF_TX

// foreground colors
#define FG_BLACK	"\033[30m"
#define FG_RED		"\033[31m"
#define FG_GREEN	"\033[32m"
#define FG_YELLOW	"\033[33m"
#define FG_BLUE		"\033[34m"
#define FG_VIOLETT	"\033[35m"
#define FG_KOBALT	"\033[36m"
#define FG_WHITE	"\033[37m"

// background colors
#define BG_BLACK	"\033[40m"
#define BG_RED		"\033[41m"
#define BG_GREEN	"\033[42m"
#define BG_YELLOW	"\033[43m"
#define BG_BLUE		"\033[44m"
#define BG_VIOLETT	"\033[45m"
#define BG_KOBALT	"\033[46m"
#define BG_WHITE	"\033[47m"

// controls
#define CLEAR		"\033[2J"
#define CLEARLINE	"\033[K"
#define RESET_ATTR	"\033[0m"
#define BOLD		"\033[1m"
#define UNDERLINE	"\033[4m"
#define BLINK		"\033[5m"
#define INVERSE		"\033[7m"
#define INVISIBLE	"\033[8m"

// cursor
#define STORE_POS	"\033[s"
#define RESTORE_POS	"\033[u"
#define SET_POS		"\033[%d;%dH"


/* types */
typedef enum{
	// control codes
	ESC_INVAL = 0,
	ESC_PARTIAL,

	// terminal escape codes
	ESC_BACKSPACE,
	ESC_BELL,
	ESC_CARRIAGE_RETURN,
	ESC_DELETE,
	ESC_FORM_FEED,
	ESC_NEWLINE,
	ESC_TAB,
	ESC_VERT_TAB,
	ESC_END_OF_TEXT,
	ESC_CTRL_C = ESC_END_OF_TEXT,
	ESC_END_OF_TX,
	ESC_CTRL_D = ESC_END_OF_TX,

	ESC_CURSOR_MOVE_UP,
	ESC_CURSOR_MOVE_UP_HOME,
	ESC_CURSOR_MOVE_DOWN,
	ESC_CURSOR_MOVE_DOWN_HOME,
	ESC_CURSOR_MOVE_LEFT,
	ESC_CURSOR_MOVE_RIGHT,
	ESC_CURSOR_MOVE_HOME,
	ESC_CURSOR_SCROLL_DOWN,
	ESC_CURSOR_SCROLL_UP,
	ESC_CURSOR_SET_COLUMN,
	ESC_CURSOR_RESTORE,
	ESC_CURSOR_SAVE,

	ESC_ERASE_IN_DISPLAY,
	ESC_ERASE_IN_LINE,

	ESC_SCROLL_SET,
	ESC_WRAP_CLR,
	ESC_WRAP_SET,
} esc_t;

typedef struct esc_state_t{
	esc_t (*hdlr)(struct esc_state_t *esc, char c);

	unsigned int val[2];
	uint8_t nval;
} esc_state_t;


/* prototypes */
void esc_init(esc_state_t *esc);
bool esc_active(esc_state_t *esc);

esc_t esc_parse(esc_state_t *esc, char c);


#endif // SYS_ESCAPE_H
