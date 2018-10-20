#include <sys/errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <cmd/cmd.h>


/* local functions */
static int exec(int argc, char **argv){
	if(argc < 2){
		printf("usage: %s <file>\n", argv[0]);
		return -1;
	}

	if(unlink(argv[1]) != 0){
		printf("error \"%s\"\n", strerror(errno));
		return -1;
	}

	return 0;
}

command("rm", exec);
