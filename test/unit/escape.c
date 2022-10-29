/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/types.h>
#include <sys/escape.h>
#include <test/test.h>


/* macros */
#define VALUES(...)	(unsigned int []){ __VA_ARGS__ }


/* local functions */
TEST(esc_active){
	int r = 0;
	esc_state_t esc;


	esc_init(&esc);

	r += TEST_INT_EQ(esc_active(&esc), false);

	esc_parse(&esc, 'c');		r += TEST_INT_EQ(esc_active(&esc), false);
	esc_parse(&esc, '\033');	r += TEST_INT_EQ(esc_active(&esc), true);
	esc_parse(&esc, 'x');		r += TEST_INT_EQ(esc_active(&esc), false);

	esc_parse(&esc, '\033');	r += TEST_INT_EQ(esc_active(&esc), true);
	esc_parse(&esc, '[');		r += TEST_INT_EQ(esc_active(&esc), true);
	esc_parse(&esc, 'D');		r += TEST_INT_EQ(esc_active(&esc), false);

	return -r;
}

int test(char const *s, esc_t code, uint8_t nval, unsigned int *values){
	int r = 0;
	esc_t e = ESC_PARTIAL;
	esc_state_t esc;


	esc_init(&esc);

	for(; *s!=0 && e!=ESC_INVAL; s++)
		e = esc_parse(&esc, *s);

	r += TEST_INT_EQ(e, code);
	r += TEST_INT_EQ(esc.nval, nval);

	for(uint8_t i=0; i<nval; i++)
		r += TEST_INT_EQ(esc.val[i], values[i]);

	return -r;
}

TEST(esc_parse){
	int r = 0;


	/* control characters */
	r += test("\t", ESC_TAB, 0, 0x0);
	r += test("\b", ESC_BACKSPACE, 0, 0x0);
	r += test("\r", ESC_CARRIAGE_RETURN, 0, 0x0);
	r += test("\n", ESC_NEWLINE, 0, 0x0);
	r += test("\v", ESC_VERT_TAB, 0, 0x0);
	r += test("\f", ESC_FORM_FEED, 0, 0x0);
	r += test("\a", ESC_BELL, 0, 0x0);
	r += test("\003", ESC_END_OF_TEXT, 0, 0x0);
	r += test("\004", ESC_END_OF_TX, 0, 0x0);
	r += test("\177", ESC_DELETE, 0, 0x0);
	r += test("\176", ESC_INVAL, 0, 0x0);

	/* single-character escape codes */
	r += test("\0337", ESC_CURSOR_SAVE, 0, 0x0);
	r += test("\0338", ESC_CURSOR_RESTORE, 0, 0x0);
	r += test("\033D", ESC_CURSOR_SCROLL_DOWN, 0, 0x0);
	r += test("\033M", ESC_CURSOR_SCROLL_UP, 0, 0x0);
	r += test("\033H", ESC_TAB, 0, 0x0);
	r += test("\033x", ESC_INVAL, 0, 0x0);

	/* [ escape codes */
	r += test("\033[s", ESC_CURSOR_SAVE, 0, 0x0);
	r += test("\033[u", ESC_CURSOR_RESTORE, 0, 0x0);
	r += test("\033[A", ESC_CURSOR_MOVE_UP, 0, 0x0);
	r += test("\033[B", ESC_CURSOR_MOVE_DOWN, 0, 0x0);
	r += test("\033[C", ESC_CURSOR_MOVE_RIGHT, 0, 0x0);
	r += test("\033[D", ESC_CURSOR_MOVE_LEFT, 0, 0x0);
	r += test("\033[E", ESC_CURSOR_MOVE_DOWN_HOME, 0, 0x0);
	r += test("\033[F", ESC_CURSOR_MOVE_UP_HOME, 0, 0x0);
	r += test("\033[G", ESC_CURSOR_SET_COLUMN, 0, 0x0);
	r += test("\033[f", ESC_CURSOR_MOVE_HOME, 0, 0x0);
	r += test("\033[H", ESC_CURSOR_MOVE_HOME, 0, 0x0);
	r += test("\033[K", ESC_ERASE_IN_LINE, 0, 0x0);
	r += test("\033[J", ESC_ERASE_IN_DISPLAY, 0, 0x0);
	r += test("\033[r", ESC_SCROLL_SET, 0, 0x0);
	r += test("\033[7h", ESC_WRAP_SET, 1, VALUES(7));
	r += test("\033[8h", ESC_INVAL, 1, VALUES(8));
	r += test("\033[7l", ESC_WRAP_CLR, 1, VALUES(7));
	r += test("\033[8l", ESC_INVAL, 1, VALUES(8));
	r += test("\033[x", ESC_INVAL, 0, 0x0);

	/* [ escape codes with values */
	r += test("\033[10A", ESC_CURSOR_MOVE_UP, 1, VALUES(10));
	r += test("\033[11B", ESC_CURSOR_MOVE_DOWN, 1, VALUES(11));
	r += test("\033[12C", ESC_CURSOR_MOVE_RIGHT, 1, VALUES(12));
	r += test("\033[13D", ESC_CURSOR_MOVE_LEFT, 1, VALUES(13));
	r += test("\033[20E", ESC_CURSOR_MOVE_DOWN_HOME, 1, VALUES(20));
	r += test("\033[21F", ESC_CURSOR_MOVE_UP_HOME, 1, VALUES(21));
	r += test("\033[30G", ESC_CURSOR_SET_COLUMN, 1, VALUES(30));
	r += test("\033[40;41f", ESC_CURSOR_MOVE_HOME, 2, VALUES(40, 41));
	r += test("\033[42;43H", ESC_CURSOR_MOVE_HOME, 2, VALUES(42, 43));
	r += test("\033[50K", ESC_ERASE_IN_LINE, 1, VALUES(50));
	r += test("\033[51J", ESC_ERASE_IN_DISPLAY, 1, VALUES(51));
	r += test("\033[40;41;42f", ESC_INVAL, 2, VALUES(40, 41));

	return -r;
}
