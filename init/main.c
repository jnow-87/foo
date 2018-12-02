/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/escape.h>
#include <shell/shell.h>


/* global functions */
int main(int argc, char **argv){
	/* shell */
	shell(FG_BLUE "::: " RESET_ATTR);

	return 0;
}
