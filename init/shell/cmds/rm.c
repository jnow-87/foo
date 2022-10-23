/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/errno.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <shell/shell.h>
#include <shell/cmd.h>


/* local functions */
static int exec(int argc, char **argv){
	stat_t f_stat;


	if(argc < 2)
		return cmd_help(argv[0], "<file>", "missing arguments", 0);

	if(stat(argv[1], &f_stat) != 0)
		return -ERROR("stat");

	if(f_stat.type == FT_DIR){
		printf("%s is a directory, do you want to delete it?\n", argv[1]);

		if(fgetc(stdin) != 'y')
			return 0;
	}

	if(unlink(argv[1]) != 0)
		return -ERROR("unlink");

	return 0;
}

command("rm", exec);
