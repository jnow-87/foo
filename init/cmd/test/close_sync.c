/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <cmd/test/test.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


/* local functions */
/**
 * \brief	test to verify that close() finishes all outstanding operations
 * 			for the given file descriptor
 */
static int exec(void){
	FILE *fp;
	int e;
	char buf[20];
	f_mode_t f_mode;


	fp = fopen("dummy", "w");

	/* set non-blocking mode for file descriptor */
	e = fcntl(fp->fileno, F_MODE_GET, &f_mode, sizeof(f_mode));
	printf("%#x %#x %#x\n", e, errno, f_mode);

	f_mode |= O_NONBLOCK;
	e = fcntl(fp->fileno, F_MODE_SET, &f_mode, sizeof(f_mode));
	printf("%#x %#x %#x\n", e, errno, f_mode);

	/* issue some writes */
	e = write(fp->fileno, "123", 1);
	printf("write (%d)\n", e);

	e = write(fp->fileno, "456", 2);
	printf("write (%d)\n", e);

	e = write(fp->fileno, "789", 3);
	printf("write (%d)\n", e);

	e = write(fp->fileno, "1234", 4);
	printf("write (%d)\n", e);

	/* close */
	fclose(fp);

	/* read back */
	fp = fopen("dummy", "r");

	e = read(fp->fileno, buf, 20);
	buf[e] = 0;
	printf("read (%d): %s\n", e, buf);

	fclose(fp);

	/* check */
	if(strcmp(buf, "1457891234") != 0)
		return 1;

	return 0;
}

test("close-sync", exec, "test if close() syncs all outstanding file operations");
