/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/term.h>
#include <sys/escape.h>
#include <test/test.h>


/* local functions */
/**
 * \brief	test to demonstrate non/blocking operation of stdin
 */
TEST_LONG(tty, "test tty non/blocking io"){
	size_t i = 0,
		   n = 0;
	char buf[20];
	term_cfg_t cfg;
	f_mode_t f_mode;


	/* configure terminal */
	ioctl(0, IOCTL_CFGRD, &cfg);
	cfg.iflags |= TIFL_CRNL;
	cfg.oflags |= TOFL_NLCR;
	cfg.lflags &= ~TLFL_ECHO;
	ioctl(0, IOCTL_CFGWR, &cfg);

	/* get stdin file mode */
	if(fcntl(0, F_MODE_GET, &f_mode, sizeof(f_mode_t)) != 0){
		printf(FG("error ", RED) "can't read stdin file mode \"%s\"\n", strerror(errno));
		return -1;
	}

	/* main loop */
	printf(
		"use 'b' to toggle between blocking and non-blocking input\n"
		"use 'q' to quit the test\n"
	);

	buf[0] = 'i';

	while(1){
		// read
		reset_errno();
		n = fread(buf, 1, stdin);
		i++;

		// error handling
		if(errno){
			printf(FG("error ", RED) "read \"%s\" -", strerror(errno));
			reset_errno();

			continue;
		}

		if(n == 0)
			continue;

		// echo
		buf[n] = 0;
		printf("%u read: \"%s\" %u\n", i, buf, n);

		// special character handling
		if(buf[0] == 'b'){
			i = 0;
			f_mode ^= O_NONBLOCK;

			printf("toggle blocking mode (set mode: %#x)\n", f_mode);

			if(fcntl(0, F_MODE_SET, &f_mode, sizeof(f_mode_t)) != 0)
				printf(FG("error ", RED) "fcntl failed \"%s\"\n", strerror(errno));
		}
		else if(buf[0] == 'q')
			break;
	}

	/* restore blocking mode */
	f_mode &= ~O_NONBLOCK;
	(void)fcntl(0, F_MODE_SET, &f_mode, sizeof(f_mode_t));

	return 0;
}
