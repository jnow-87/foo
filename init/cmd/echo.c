#include <stdio.h>


/* global functions */
int echo(int argc, char **argv){
	int i;


	for(i=1; i<argc; i++){
		fputs(argv[i], stdout);
		fputc(' ', stdout);
	}

	fputc('\n', stdout);

	return 0;
}
