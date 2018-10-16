#include <sys/errno.h>
#include <stdio.h>
#include <string.h>
#include <cmd/cmd.h>


/* local functions */
static int exec(int argc, char **argv){
	int c;
	FILE *fp;


	if(argc < 2){
		printf("usage: %s <ifile>\n", argv[0]);
		return -1;
	}

	fp = fopen(argv[1], "r");

	if(fp == 0x0){
		printf("open \"%s\" failed \"%s\"\n", argv[1], strerror(errno));
		return -1;
	}

	while(1){
		c = fgetc(fp);

		if(c == EOF)
			break;

		fputc((char)c, stdout);
	}

	if(errno)
		printf("error %s\n", strerror(errno));

	fclose(fp);

	return 0;
}

command("cat", exec);
