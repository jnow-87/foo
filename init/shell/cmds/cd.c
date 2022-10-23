/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <unistd.h>
#include <shell/shell.h>
#include <shell/cmd.h>


/* local functions */
static int exec(int argc, char **argv){
	char *path = "/";


	if(argc > 1)
		path = argv[1];

	if(chdir(path) != 0)
		return -ERROR("chdir");

	return 0;
}

command("cd", exec);
