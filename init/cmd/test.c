#include <stdio.h>
#include <cmd/cmd.h>


/* local functions */
static int exec(int argc, char **argv){
	printf("test dummy\n");
	return 0;
}

command("test", exec);
