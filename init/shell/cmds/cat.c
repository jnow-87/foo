/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <shell/cmd.h>


/* macros */
#define ARGS	"<ifile>"
#define OPTS \
	"-b", "assume binary data from <ifile>", \
	"-n <blocks>", "number of blocks to read", \
	"-s <block-size>", "number of bytes per block (default 1)", \
	"-o <offset>", "offset into file (default 0)", \
	"-h", "print this help message"


/* local functions */
static int exec(int argc, char **argv){
	ssize_t i;
	char opt;
	size_t n,
		   bs,
		   decr;
	ssize_t r;
	int offset;
	char *buf;
	bool binary;
	int fd;
	stat_t f_stat;
	seek_t f_seek;


	buf = 0x0;
	n = 0;
	bs = 1;
	decr = 1;
	offset = 0;
	binary = false;

	/* check options */
	while((opt = getopt(argc, argv, "bn:s:o:h")) != -1){
		switch(opt){
		case 'b':	binary = true; break;
		case 'n':	n = atoi(optarg); break;
		case 's':	bs = atoi(optarg); break;
		case 'o':	offset = atoi(optarg); break;
		case 'h':	return CMD_HELP(argv[0], 0x0);
		default:	return CMD_HELP(argv[0], "");
		}
	}

	if(optind >= argc)
		return CMD_HELP(argv[0], "missing input file");

	/* open file */
	fd = open(argv[optind], O_RDONLY);

	if(fd < 0){
		fprintf(stderr, "open \"%s\" failed \"%s\"\n", argv[optind], strerror(errno));
		goto end_0;
	}

	// get file size
	if(n == 0){
		if(fstat(fd, &f_stat) != 0)
			goto end_1;

		n = f_stat.size;
	}

	if(n == 0){
		// setup infinite read loop
		decr = 0;
		n = 1;
	}

	// seek into file
	f_seek.offset = offset;
	f_seek.whence = SEEK_SET;

	if(offset && fcntl(fd, F_SEEK, &f_seek, sizeof(seek_t)) != E_OK){
		fprintf(stderr, "seek failed \"%s\"\n", strerror(errno));
		goto end_1;
	}

	/* allocate buffer */
	buf = malloc(bs + 1);

	if(buf == 0x0){
		fprintf(stderr, "unable to allocate buffer memory \"%s\"\n", strerror(errno));
		goto end_1;
	}

	/* read */
	for(; n>0; n-=decr){
		r = read(fd, buf, bs);

		if(r <= 0 || errno)
			break;

		buf[r] = 0;

		if(binary){
			for(i=0; i<r; i++)
				printf("%x", (int)(buf[i] & 0xff));
		}
		else
			fprintf(stdout, buf);
	}

	if(errno)
		fprintf(stderr, "error \"%s\"\n", strerror(errno));

	/* cleanup */
	free(buf);

end_1:
	close(fd);

end_0:
	return errno;
}

command("cat", exec);
