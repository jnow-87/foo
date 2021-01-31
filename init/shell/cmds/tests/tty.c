/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/term.h>
#include <sys/uart.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <shell/cmds/tests/test.h>


/* local functions */
/**
 * \brief	test to demonstrate non/blocking operation of stdin
 */
static int exec(void){
	size_t i,
		   n;
	char buf[20];
	uart_cfg_t cfg;
	term_err_t err;
	f_mode_t f_mode;


	/* disable terminal echo */
	ioctl(0, IOCTL_CFGRD, &cfg, sizeof(uart_cfg_t));
	cfg.flags &= ~TERM_FLAG_ECHO;
	ioctl(0, IOCTL_CFGWR, &cfg, sizeof(uart_cfg_t));

	/* get stdin file mode */
	if(fcntl(0, F_MODE_GET, &f_mode, sizeof(f_mode_t)) != 0){
		ERROR("can't read stdin file mode \"%s\"\n", strerror(errno));
		return -1;
	}

	/* main loop */
	printf(
		"use 'b' to toggle between blocking and non-blocking input\n"
		"use 'q' to quit the test\n"
	);

	buf[0] = 'i';
	errno = E_OK;
	i = 0;
	n = 0;

	while(1){
		// read
		errno = E_OK;
		n = fread(buf, 1, stdin);
		i++;

		// error handling
		if(errno){
			ERROR("read \"%s\" -", strerror(errno));
			errno = E_OK;

			ioctl(0, IOCTL_STATUS, &err, sizeof(err));

			if(errno == E_OK){
				fprintf(stderr, "%s%s%s%s\n",
					(err & TERM_ERR_DATA_OVERRUN ? " data overrun" : ""),
					(err & TERM_ERR_PARITY ? " parity error" : ""),
					(err & TERM_ERR_FRAME ? " frame error" : ""),
					(err & TERM_ERR_RX_FULL ? " rx queue full" : "")
				);
			}
			else
				fprintf(stderr, "\nioctl error \"%s\"\n", strerror(errno));

			errno = E_OK;
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
				ERROR("fcntl failed \"%s\"\n", strerror(errno));
		}
		else if(buf[0] == 'q')
			break;
	}

	/* restore blocking mode */
	f_mode &= ~O_NONBLOCK;
	(void)fcntl(0, F_MODE_SET, &f_mode, sizeof(f_mode_t));

	return 0;
}

test("tty", exec, "tty non/blocking io");
