/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/dirent.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/escape.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <shell/shell.h>
#include <shell/cmd.h>



/* local functions */
static int exec(int argc, char **argv){
	char *path = ".";
	int dir;
	dir_ent_t entry;
	stat_t f_stat;


	if(argc > 1)
		path = argv[1];

	dir = open(path, O_RDONLY);

	if(dir < 0)
		return -ERROR("opening %s", path);

	while(1){
		if(read(dir, &entry, sizeof(dir_ent_t)) <= 0){
			if(errno)
				ERROR("reading");

			break;
		}

		statat(path, entry.name, &f_stat);

		if(f_stat.type == FT_DIR)
			fputs(FG_BLUE, stdout);

		printf("%s%s\n", entry.name, (f_stat.type == FT_DIR ? "/" RESET_ATTR : ""));
	}

	close(dir);

	return 0;
}

command("ls", exec);
