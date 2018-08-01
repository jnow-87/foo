#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "avrconfig.h"


/* external functions */
int config_watchdog(void);


/* global variables */
arg_t arg = { 0 };


/* local/static prototypes */
static int arg_parse(int argc, char **argv);


/* global functions */
int main(int argc, char **argv){
	int r;


	if(arg_parse(argc, argv) < 0)
		return 1;

	printf("generating avr config header \"%s\"\n", arg.ofile_name);

	/* compute config variables */
	r = 0;

	r |= config_watchdog();

	/* close output file */
	fclose(arg.ofile);

	if(r == 0)
		return 0;

	unlink(arg.ofile_name);

	return 3;
}


/* local functions */
static int arg_parse(int argc, char **argv){
	int i;


	for(i=1; i<argc; i++){
		if(argv[i][0] == '-'){
			switch(argv[i][1]){
			case 'v':
				arg.verbose = 1;
				break;

			default:
				fprintf(stderr, "invalid argument \"%s\"\n", argv[i]);
				return -1;
			}
		}
		else{
			arg.ofile_name = argv[i];
			arg.ofile = fopen(argv[i], "w");

			if(arg.ofile == 0)
				fprintf(stderr, "unable to open output header \"%s\"\n", strerror(errno));
		}
	}

	if(arg.ofile == 0){
		fprintf(stderr, "missing output file\n");
		return -1;
	}

	return 0;
}
