/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <shell/cmd.h>


/* local/static prototyes */
static int help(char const *prog_name, char const *msg, ...);


/* local functions */
static int exec(int argc, char **argv){
	int i;
	size_t n;
	uint8_t c;
	bool binary;
	FILE *fp;
	stat_t f_stat;


	n = 0;
	binary = false;

	/* check options */
	for(i=1; i<argc && argv[i][0]=='-'; i++){
		switch(argv[i][1]){
		case 'b':
			binary = true;
			break;

		case 'n':
			if(i + 1 >= argc)
				return help(argv[0], "missing argument to option '-n'");

			n = atoi(argv[++i]);
			break;

		default:
			return help(argv[0], "invalid option '%s'\n", argv[i]);
		}
	}

	if(i >= argc)
		return help(argv[0], "missing input file");

	/* read */
	// open file
	fp = fopen(argv[i], "r");

	if(fp == 0x0){
		printf("open \"%s\" failed \"%s\"\n", argv[i], strerror(errno));
		return 1;
	}

	// get file size
	if(n == 0){
		if(fstat(fileno(fp), &f_stat) != 0)
			goto end;

		n = f_stat.size;
	}

	// read
	for(; n>0; n--){
		c = fgetc(fp);

		if(c == EOF)
			break;

		if(binary)	printf("%x", (int)(c & 0xff));
		else		fputc(c, stdout);
	}


	/* cleanup */
end:
	if(errno)
		printf("error %s\n", strerror(errno));

	fclose(fp);

	return 0;
}

command("cat", exec);

static int help(char const *prog_name, char const *msg, ...){
	va_list lst;


	if(msg){
		va_start(lst, msg);
		vfprintf(stderr, msg, lst);
		va_end(lst);
		fputs("\n", stderr);
	}

	fprintf(stderr,
		"usage: %s <options> <ifile>\n"
		"\noptions:\n"
		"%15.15s    %s\n"
		"%15.15s    %s\n"
		, prog_name
		, "-n", "number of bytes to read"
		, "-b", "assume binary data from <ifile>"
	);

	return (msg ? 1 : 0);
}
