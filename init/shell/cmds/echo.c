/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/stdarg.h>
#include <sys/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <shell/cmd.h>


/* macros */
#define ARGS	"<args>"
#define OPTS \
	"-b", "convert <args> to binary representation", \
	"-n", "skip trailing newline", \
	"-h", "print this help message"


/* local functions */
static int exec(int argc, char **argv){
	int i;
	char opt;
	uint8_t x;
	bool binary,
		 newline;


	binary = false;
	newline = true;

	/* check options */
	while((opt = getopt(argc, argv, "bnh")) != -1){
		switch(opt){
		case 'b':	binary = true; break;
		case 'n':	newline = false; break;
		case 'h':	return CMD_HELP(argv[0], 0x0);
		default:	return CMD_HELP(argv[0], "");
		}
	}

	/* echo non-option arguments */
	for(i=optind; i<argc; i++){
		if(binary){
			x = strtol(argv[i], 0x0, 0);
			fwrite(&x, sizeof(x), stdout);
		}
		else{
			fputs(argv[i], stdout);

			if(i + 1 < argc)
				fputc(' ', stdout);
		}

		if(errno)
			break;
	}

	if(!binary && newline)
		fputc('\n', stdout);

	fflush(stdout);

	if(errno)
		fprintf(stderr, "error \"%s\"\n", strerror(errno));

	return errno;
}

command("echo", exec);
