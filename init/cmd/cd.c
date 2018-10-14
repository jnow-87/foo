#include <sys/errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>


/* global functions */
int cd(int argc, char **argv){
	char *path;


	path = "/";

	if(argc > 1)
		path = argv[1];

	if(chdir(path) != 0){
		printf("error \"%s\"\n", strerror(errno));
		return -1;
	}

	return 0;
}
