#include <stdio.h>
#include <cmd/cmd.h>


/* local functions */
static int exec(int argc, char **argv){
	int i;


	for(i=1; i<argc; i++){
		fputs(argv[i], stdout);
		fputc(' ', stdout);
	}

	fputc('\n', stdout);

	return 0;
}

command("echo", exec);
