#include <sys/dirent.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/escape.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <cmd/cmd.h>



/* local functions */
static int exec(int argc, char **argv){
	int dir;
	char *path;
	dir_ent_t entry;
	stat_t f_stat;


	path = ".";

	if(argc > 1)
		path = argv[1];

	dir = open(path, O_RDONLY);

	if(dir < 0){
		printf("open \"%s\" failed \"%s\"\n", argv[1], strerror(errno));
		return -1;
	}

	while(1){
		if(read(dir, &entry, sizeof(dir_ent_t)) <= 0){
			if(errno)
				printf("read error \"%s\"\n", strerror(errno));

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
