/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <shell/cmd.h>


/* local functions */
static int exec(int argc, char **argv){
	char *path;


	path = "/";

	if(argc > 1)
		path = argv[1];

	if(chdir(path) != 0){
		fprintf(stderr, "error \"%s\"\n", strerror(errno));
		return -1;
	}

	return 0;
}

command("cd", exec);
