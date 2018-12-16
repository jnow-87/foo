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
#include <shell/cmd.h>


/* local functions */
static int exec(int argc, char **argv){
	stat_t f_stat;


	if(argc < 2){
		printf("usage: %s <file>\n", argv[0]);
		return -1;
	}

	if(stat(argv[1], &f_stat) != 0){
		printf("file not found\n");
		return -1;
	}

	if(f_stat.type == FT_DIR){
		printf("%s is a directory, do you want to delete it?\n", argv[1]);

		if(fgetc(stdin) != 'y')
			return 0;
	}

	if(unlink(argv[1]) != 0){
		printf("error \"%s\"\n", strerror(errno));
		return -1;
	}

	return 0;
}

command("rm", exec);
