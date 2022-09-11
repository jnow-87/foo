/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <sys/string.h>
#include <shell/cmd.h>


/* local functions */
static int exec(int argc, char **argv){
	int status;


	status = 0;

	if(argc > 1)
		status = atoi(argv[1]);

	exit(status);

	return 1;
}

command("exit", exec);
