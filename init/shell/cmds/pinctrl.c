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
#include <sys/ioctl.h>
#include <sys/ringbuf.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <shell/cmd.h>


static size_t sleep_ms = 100;
static size_t sleep_us = 100;


void out(void){
	printf("sleep: %zu %zu us\n", sleep_ms, sleep_us);
}

/* local functions */
/**
 *	\brief	sample given input pin and output its level
 */
static int exec(int argc, char **argv){
	char c;
	uint8_t lvl;
	int pin_fd;
	f_mode_t f_mode;


	if(argc < 2){
		printf("usage: %s <pin device>\n", argv[0]);
		return -1;
	}

	/* init device */
	pin_fd = open(argv[1], O_RDWR);

	if(pin_fd < 0){
		printf("open \"%s\" failed \"%s\"\n", argv[1], strerror(errno));
		return -1;
	}

	/* make stdin non-blocking */
	fcntl(0, F_MODE_GET, &f_mode, sizeof(f_mode_t));
	f_mode |= O_NONBLOCK;
	fcntl(0, F_MODE_SET, &f_mode, sizeof(f_mode_t));

	/* usage */
	printf(
		"'m/M' de/increase sleep interval [ms]\n"
		"'u/U' de/increase sleep interval [us]\n"
		"'q' quit\n\n"
	);

	/* main loop */
	c = 0;
	lvl = 0;


	while(c != 'q'){
		// check user input
		if(read(0, &c, 1) == 1){
			switch(c){
			case 'q':	continue;
			case 'm':	sleep_ms -= 10; out(); break;
			case 'M':	sleep_ms += 10; out(); break;
			case 'u':	sleep_us -= 10; out(); break;
			case 'U':	sleep_us += 10; out(); break;
			}
		}

		usleep(sleep_us);
//		sleep(sleep_ms, sleep_us);
		write(pin_fd, &lvl, 1);
		lvl ^= 1;
	}

	/* restore stdin blocking mode */
	f_mode &= ~O_NONBLOCK;
	fcntl(0, F_MODE_SET, &f_mode, sizeof(f_mode_t));

	fputc('\n', stdout);

	return 0;
}

command("pinctrl", exec);
