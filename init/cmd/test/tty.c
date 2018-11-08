/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <cmd/test/test.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/term.h>
#include <unistd.h>
#include <stdio.h>





/* local functions */
/**
 * \brief	test to demonstrate non/blocking operation of stdin
 */
static int exec(void){
	size_t i,
		   n;
	int e;
	char buf[20];
	term_cfg_t tc;
	term_err_t err;
	f_mode_t f_mode;


	/* disable terminal echo */
	ioctl(0, IOCTL_CFGRD, &tc, sizeof(term_cfg_t));
	tc.flags &= ~TF_ECHO;
	ioctl(0, IOCTL_CFGWR, &tc, sizeof(term_cfg_t));

	/* get stdin file mode */
	e = fcntl(0, F_MODE_GET, &f_mode, sizeof(f_mode_t));

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
		n = fread(buf, 1, stdin);
		i++;

		// error handling
		if(errno){
			printf("read error %#x -", errno);
			errno = E_OK;

			ioctl(0, IOCTL_STATUS, &err, sizeof(err));

			if(errno == E_OK){
				printf("%s%s%s%s\n",
					(err & TE_DATA_OVERRUN ? " data overrun" : ""),
					(err & TE_PARITY ? " parity error" : ""),
					(err & TE_FRAME ? " frame error" : ""),
					(err & TE_RX_FULL ? " rx queue full" : "")
				);
			}
			else
				printf("\nioctl error %#x\n", errno);

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

			e = fcntl(0, F_MODE_SET, &f_mode, sizeof(f_mode_t));

			if(e != 0)
				printf("fcntl error %#x %#x\n", e, errno);
		}
		else if(buf[0] == 'q')
			break;
	}

	return 0;
}

test("tty", exec, "tty non/blocking io");
