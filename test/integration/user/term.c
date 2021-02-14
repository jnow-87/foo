/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdbool.h>
#include <unistd.h>
#include <termios.h>


/* local/static prototypes */
static int get_defaults(struct termios *attr);


/* static variables */
static bool default_attr_set = false;
static struct termios default_attr;


/* global functions */
void term_default(void){
	if(default_attr_set)
		(void)tcsetattr(0, TCSANOW, &default_attr);
}

int term_noncanon(void){
	struct termios attr;


	if(get_defaults(&attr) != 0)
		return -1;

	attr.c_lflag &= ~(ICANON | ECHO);

	if(tcsetattr(0, TCSANOW, &attr) != 0)
		return -1;

	return 0;
}


/* local functions */
static int get_defaults(struct termios *attr){
	if(!default_attr_set){
		if(tcgetattr(0, attr) != 0)
			return -1;

		default_attr = *attr;
		default_attr_set = true;
	}

	*attr = default_attr;

	return 0;
}
