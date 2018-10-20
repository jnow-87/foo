#include <sys/dirent.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <cmd/cmd.h>



/* local functions */
static int exec(int argc, char **argv){
	int dir;
	char *path;
	dir_ent_t entry;


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

		printf("%s\n", entry.name);
	}

	close(dir);

	return 0;
}

command("ls", exec);
