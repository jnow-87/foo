/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/escape.h>
#include <shell/cmd.h>


/* macros */
#define ARGS	"<ifile>"
#define OPTS \
	"-n <blocks>", "number of blocks to read", \
	"-s <block-size>", "number of bytes per block (default 1)", \
	"-o <offset>", "offset into file (default 0)", \
	"-x", "hex dump mode", \
	"-h", "print this help message"

#define DEFAULT_OPTS() (opts_t){ \
	.hexdump = false, \
	.length = -1, \
	.blocksize = 8, \
	.offset = 0, \
}

#define ERROR(file, msg)({ \
	fprintf(stderr, "error:%s: %s: %s\n", file, msg, strerror(errno)); \
	-1; \
})


/* types */
typedef struct{
	bool hexdump;
	ssize_t length;
	size_t blocksize,
		   offset;
} opts_t;


/* local/static prototypes */
static int cat(char const *file);
static int prepare(char const *file);
static int readwrite(int fd, char const *file, bool interactive);


/* static variables */
static opts_t opts;


/* local functions */
static int exec(int argc, char **argv){
	char opt;


	opts = DEFAULT_OPTS();

	/* parse options */
	while((opt = getopt(argc, argv, "n:s:o:xh")) != -1){
		switch(opt){
		case 'n':	opts.length = atoi(optarg); break;
		case 's':	opts.blocksize = atoi(optarg); break;
		case 'o':	opts.offset = atoi(optarg); break;
		case 'x':	opts.hexdump = true; break;
		case 'h':	return CMD_HELP(argv[0], 0x0);
		default:	return CMD_HELP(argv[0], "");
		}
	}

	/* cat */
	if(optind == argc)
		return -readwrite(0, "stdin", true);

	for(int i=optind; i<argc; i++){
		if(cat(argv[i]) != 0)
			return 1;
	}

	return 0;
}

command("cat", exec);


static int cat(char const *file){
	int fd,
		r;


	fd = prepare(file);

	if(fd < 0)
		return -1;

	r = readwrite(fd, file, false);
	close(fd);

	return r;
}

static int prepare(char const *file){
	int fd;
	seek_t seek;


	fd = open(file, O_RDONLY);

	if(fd < 0)
		goto err_0;

	if(opts.offset > 0){
		seek.offset = opts.offset;
		seek.whence = SEEK_SET;

		if(fcntl(fd, F_SEEK, &seek, sizeof(seek_t)) != 0)
			goto err_1;
	}

	return fd;


err_1:
	close(fd);

err_0:
	return ERROR(file, "open");
}

static int readwrite(int fd, char const *file, bool interactive){
	uint8_t buf[opts.blocksize];
	ssize_t r;


	while(opts.length > 0 || opts.length == -1){
		/* read */
		r = read(fd, buf, opts.blocksize);

		if(r == 0 || (interactive && (buf[0] == CTRL_C || buf[0] == CTRL_D)))
			break;

		if(r < 0 || errno)
			return ERROR(file, "read");

		/* write */
		if(opts.hexdump){
			for(ssize_t i=0; i<r; i++)
				printf("%.2hhx", buf[i]);

			fflush(stdout);
		}
		else if(write(1, buf, r) != r)
			return ERROR(file, "write");

		if(opts.length > 0)
			opts.length--;
	}

	return 0;
}
