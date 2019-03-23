/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/types.h>
#include <sys/errno.h>
#include <sys/term.h>
#include <sys/escape.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <shell/cmd.h>


/* macros */
#define LINE_LEN	40


/* local/static prototypes */
static void update_sample_interval(size_t *cur, int inc);
static void out(char c, size_t sample_int);


/* local functions */
/**
 *	\brief	sample PINA0 and output its level
 */
static int exec(int argc, char **argv){
	int fd;
	uint8_t c;
	size_t sample_int;
	f_mode_t f_mode;


	if(argc < 2){
		printf("usage: %s <pin device>\n", argv[0]);
		return -1;
	}

	/* get PINA0 file descriptor */
	fd = open(argv[1], O_RDONLY);

	if(fd < 0){
		printf("open \"%s\" failed \"%s\"\n", argv[1], strerror(errno));
		return -1;
	}

	/* make stdin non-blocking */
	fcntl(0, F_MODE_GET, &f_mode, sizeof(f_mode_t));
	f_mode |= O_NONBLOCK;
	fcntl(0, F_MODE_SET, &f_mode, sizeof(f_mode_t));

	/* usage */
	printf(
		"'+/-' de/increase sample interval\n"
		"'q' quit\n\n"
	);

	/* store cursor position */
	printf(STORE_POS);
	fflush(stdout);

	/* main loop */
	sample_int = 100;
	c = 0;

	while(c != 'q'){
		// sample pin
		if(read(fd, &c, 1) != 1)
			continue;

		if(c == 0)	out('_', sample_int);
		else		out('-', sample_int);

		// check user input
		if(read(0, &c, 1) == 1){
			switch(c){
			case 'q':	continue;
			case '+':	update_sample_interval(&sample_int, 10); break;
			case '-':	update_sample_interval(&sample_int, -10); break;
			}
		}

		sleep(sample_int, 0);
	}

	/* restore stdin blocking mode */
	f_mode &= ~O_NONBLOCK;
	fcntl(0, F_MODE_SET, &f_mode, sizeof(f_mode_t));

	fputc('\n', stdout);

	return 0;
}

command("pinsampler", exec);

/**
 * \brief	modify the sample interval based on inc
 * 			min value: 10
 * 			max value: 300
 */
static void update_sample_interval(size_t *cur, int inc){
	size_t t;


	t = *cur + inc;

	if(inc > 0 && t > 300)						*cur = 300;
	else if(inc < 0 && (t < 10 || t > *cur))	*cur = 10;
	else										*cur = t;
}

/**
 * \brief	output the given character to a scolling line
 */
static void out(char c ,size_t sample_int){
	static char line[LINE_LEN];
	static unsigned int h = 0;


	line[h++] = c;

	printf(CLEARLINE "interval [ms]: %-5u\n", (unsigned int)sample_int);
	fflush(stdout);

	write(1, line + h, LINE_LEN - h);
	write(1, line, h);

	fputs("\033[1F", stdout);
	fflush(stdout);

	if(h == LINE_LEN)
		h = 0;
}
