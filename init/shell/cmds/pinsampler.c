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


/* macros */
#define LINE_LEN	40


/* local/static prototypes */
static void pin_change_hdlr(signal_t sig);
static void update_sample_interval(size_t *cur, int inc);
static void out(char c, size_t sample_interval);


/* static variables */
static int pin_fd = -1;
static char pin_lvl = 0;
static char buf_data[10];
static ringbuf_t buf = RINGBUF_INITIALISER(buf_data, 10);


/* local functions */
/**
 *	\brief	sample given input pin and output its level
 */
static int exec(int argc, char **argv){
	uint8_t c;
	size_t sample_interval;
	bool use_int;
	f_mode_t f_mode;
	signal_t sig;


	if(argc < 2){
		printf("usage: %s <pin device>\n", argv[0]);
		return -1;
	}

	/* init device */
	pin_fd = open(argv[1], O_RDONLY);

	if(pin_fd < 0){
		printf("open \"%s\" failed \"%s\"\n", argv[1], strerror(errno));
		return -1;
	}

	// try registering change interrupt
	use_int = 0;
	sig = SIG_USR1;

	if(ioctl(pin_fd, IOCTL_CFGWR, &sig, sizeof(signal_t)) == 0){
		if(signal(sig, pin_change_hdlr) == pin_change_hdlr){
			use_int = 1;
			printf("use pin interrupt for sampling\n");
		}
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
	sample_interval = 1000;
	c = 0;

	while(c != 'q'){
		// sample pin
		if(!use_int && read(pin_fd, &pin_lvl, 1) != 1)
			continue;

		if(use_int)
			ringbuf_read(&buf, &pin_lvl, 1);

		if(pin_lvl == 0)	out('_', sample_interval);
		else				out('-', sample_interval);

		// check user input
		if(read(0, &c, 1) == 1){
			switch(c){
			case 'q':	continue;
			case '+':	update_sample_interval(&sample_interval, 10); break;
			case '-':	update_sample_interval(&sample_interval, -10); break;
			}
		}

		sleep(sample_interval, 0);
	}

	/* restore stdin blocking mode */
	f_mode &= ~O_NONBLOCK;
	fcntl(0, F_MODE_SET, &f_mode, sizeof(f_mode_t));

	fputc('\n', stdout);

	return 0;
}

command("pinsampler", exec);

/**
 * \brief	signal handler for pin change interrupt
 */
static void pin_change_hdlr(signal_t sig){
	read(pin_fd, &pin_lvl, 1);

	if(pin_lvl == 0)	out('_', 100);
	else				out('-', 100);

	ringbuf_write(&buf, &pin_lvl, 1);
}

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
static void out(char c ,size_t sample_interval){
	static char line[LINE_LEN];
	static unsigned int h = 0;


	line[h++] = c;

	printf(CLEARLINE "interval [ms]: %-5u\n", (unsigned int)sample_interval);
	fflush(stdout);

	write(1, line + h, LINE_LEN - h);
	write(1, line, h);

	fputs("\033[1F", stdout);
	fflush(stdout);

	if(h == LINE_LEN)
		h = 0;
}
