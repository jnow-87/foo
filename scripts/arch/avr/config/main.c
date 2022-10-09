/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "avrconfig.h"


/* external functions */
int config_watchdog(void);


/* global variables */
arg_t arg = { 0 };
FILE *tmp_file = 0x0;


/* local/static prototypes */
static int arg_parse(int argc, char **argv);
static int diff(char const *file0, char const *file1);


/* global functions */
int main(int argc, char **argv){
	int r;
	char *tmp_file_name;


	if(arg_parse(argc, argv) < 0)
		return 1;

	tmp_file_name = malloc(strlen(arg.ofile_name) + 5);

	if(tmp_file_name == 0x0)
		return 1;

	sprintf(tmp_file_name, "%s.tmp", arg.ofile_name);
	tmp_file = fopen(tmp_file_name, "w");

	if(tmp_file == 0x0)
		fprintf(stderr, "open temporary file failed \"%s\"\n", strerror(errno));

	if(arg.verbose)
		printf("generating temporary avr config header \"%s\"\n", tmp_file_name);

	/* compute config variables */
	r = 0;

	r |= config_watchdog();

	/* close tmp file */
	fclose(tmp_file);

	if(diff(tmp_file_name, arg.ofile_name) == 0)
		goto end;

	printf("generating avr config header \"%s\"\n", arg.ofile_name);
	r |= rename(tmp_file_name, arg.ofile_name);

end:
	unlink(tmp_file_name);

	return r;
}


/* local functions */
static int arg_parse(int argc, char **argv){
	int i;


	/* parse */
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
		}
	}

	/* validate */
	if(arg.ofile_name == 0){
		fprintf(stderr, "missing output file\n");
		return -1;
	}

	return 0;
}

static int diff(char const *file0, char const *file1){
	int fd[2];
	int i,
		r;
	char const *file[2];
	struct stat st[2];
	char *buf[2];


	file[0] = file0;
	file[1] = file1;
	fd[0] = -1;
	fd[1] = -1;
	buf[0] = 0;
	buf[1] = 0;

	r = -1;

	for(i=0; i<2; i++){
		fd[i] = open(file[i], O_RDONLY);

		if(fd[i] == -1 || stat(file[i], st + i))
			goto end_0;

		buf[i] = mmap(0x0, st[i].st_size, PROT_READ, MAP_PRIVATE, fd[i], 0);

		if(buf[i] == MAP_FAILED)
			goto end_0;
	}

	r = strcmp(buf[0], buf[1]);


end_0:
	for(i=0; i<2; i++){
		if(buf[i])
			munmap(buf[i], st[i].st_size);

		if(fd[i] != -1)
			close(fd[i]);
	}

	return r;
}
