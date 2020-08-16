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
#include <string.h>
#include <shell/cmd.h>


/* local/static prototyes */
static int help(char const *prog_name, char const *msg, ...);


/* local functions */
static int exec(int argc, char **argv){
	ssize_t i;
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

		case 's':
			if(i + 1 >= argc)
				return help(argv[0], "missing argument to option '-s'");

			bs = atoi(argv[++i]);
			break;

		case 'o':
			if(i + 1 >= argc)
				return help(argv[0], "missing argument to option '-o'");

			offset = atoi(argv[++i]);
			break;

		default:
			return help(argv[0], "invalid option '%s'\n", argv[i]);
		}
	}

	if(i >= argc)
		return help(argv[0], "missing input file");

	/* open file */
	fd = open(argv[i], O_RDONLY);

	if(fd < 0){
		fprintf(stderr, "open \"%s\" failed \"%s\"\n", argv[i], strerror(errno));
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
		"%15.15s    %s\n"
		"%15.15s    %s\n"
		, prog_name
		, "-n", "number of blocks to read"
		, "-s", "number of bytes per block (default 1)"
		, "-o", "offset into file (default 0)"
		, "-b", "assume binary data from <ifile>"
	);

	return (msg ? 1 : 0);
}
