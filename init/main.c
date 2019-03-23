/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/escape.h>
#include <sys/errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <shell/shell.h>


/* global functions */
int main(int argc, char **argv){
	int r;
	FILE *fp;
	stat_t stat;


	/* try execute first command line argument */
	if(argc > 1){
		fp = fopen(argv[1], "r");

		if(fp != 0){
			r = fstat(fileno(fp), &stat);

			if(r == 0 && stat.type == FT_REG){
				r = shell("", fp);
				printf("\"%s\" returned with %d\n", argv[1], r);
			}
			else
				printf("execution of \"%s\" failed \"%s\"\n", argv[1], (r == 0 ? "not a regular file" : strerror(errno)));

			fclose(fp);
		}
		else
			printf("open \"%s\" failed \"%s\"\n", argv[1], strerror(errno));
	}

	/* fallback shell */
	printf("executing fallback shell\n");
	shell(FG_BLUE "::: " RESET_ATTR, stdin);

	return 0;
}
