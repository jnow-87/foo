/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <shell/cmd.h>


/* local functions */
static int exec(int argc, char **argv){
	if(argc < 2)
		return cmd_help(argv[0], "<directory>", "missing arguments", 0);

	if(mkdir(argv[1]) != 0){
		fprintf(stderr, "error \"%s\"\n", strerror(errno));
		return 1;
	}

	return 0;
}

command("mkdir", exec);
