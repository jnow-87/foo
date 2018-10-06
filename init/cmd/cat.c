#include <sys/errno.h>
#include <stdio.h>


/* global functions */
int cat(int argc, char **argv){
	int c;
	FILE *fp;


	if(argc < 2){
		printf("usage: %s <ifile>\n", argv[0]);
		return -1;
	}

	fp = fopen(argv[1], "r");

	if(fp == 0x0){
		printf("open \"%s\" failed, errno %#x\n", argv[1], errno);
		return -1;
	}

	while(1){
		c = fgetc(fp);

		if(c == EOF)
			break;

		fputc((char)c, stdout);
	}

	return 0;
}
