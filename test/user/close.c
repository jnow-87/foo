/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <test/test.h>


/* local functions */
/**
 * \brief	test to verify that close() finishes all outstanding operations
 * 			for the given file descriptor
 */
TEST(close){
	int r;
	char buf[20];
	FILE *fp;
	f_mode_t f_mode;


	r = 0;

	/* init file */
	(void)unlink("dummy");
	r += TEST_PTR_NEQ(fp = fopen("dummy", "w"), 0x0);

	r += TEST_INT_EQ(fcntl(fp->fileno, F_MODE_GET, &f_mode, sizeof(f_mode)), 0);
	f_mode |= O_NONBLOCK;
	r += TEST_INT_EQ(fcntl(fp->fileno, F_MODE_SET, &f_mode, sizeof(f_mode)), 0);

	/* issue some writes */
	r += TEST_INT_EQ(write(fp->fileno, "123", 1), 1);
	r += TEST_INT_EQ(write(fp->fileno, "456", 2), 2);
	r += TEST_INT_EQ(write(fp->fileno, "789", 3), 3);
	r += TEST_INT_EQ(write(fp->fileno, "1234", 4), 4);

	/* close */
	r += TEST_INT_EQ(fclose(fp), 0);

	/* read back */
	r += TEST_PTR_NEQ(fp = fopen("dummy", "r"), 0x0);
	r += TEST_INT_EQ(read(fp->fileno, buf, 20), 10);
	r += TEST_STRN_EQ(buf, "1457891234", 10);

	r += TEST_INT_EQ(fclose(fp), 0);
	r += TEST_INT_EQ(unlink("dummy"), 0);

	return -r;
}
