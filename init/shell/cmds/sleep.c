/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <unistd.h>
#include <sys/string.h>
#include <shell/cmd.h>


/* local functions */
static int exec(int argc, char **argv){
	if(argc < 2)
		return cmd_help(argv[0], "<seconds>", "missing arguments", 0);

	return sleep(atoi(argv[1]) * 1000, 0);
}

command("sleep", exec);
