#include <config/config.h>
#include <sys/errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>


/* local/static prototypes */
int config_watchdog(FILE *header);


/* global functions */
int main(int argc, char **argv){
	char *header;
	int r;
	FILE *fp;


	if(argc < 2)
		return 1;

	header = argv[1];

	printf("generating avr config header \"%s\"\n", header);

	/* prepare output file */
	fp = fopen(header, "w");

	if(fp == 0){
		fprintf(stderr, "unable to open header \"%s\"\n", strerror(errno));
		return 2;
	}

	/* compute config variables */
	r = 0;

	r |= config_watchdog(fp);

	/* close output file */
	fclose(fp);

	if(r == 0)
		return 0;

	unlink(header);

	return 3;
}


/* local functions */
int config_watchdog(FILE *header){
	return 0;
}
