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
#include <sys/stat.h>
#include <sys/escape.h>
#include <test/test.h>


/* local functions */
/**
 * \brief	test to verify stat()
 */
TEST_LONG(stat, "test syscall stat"){
	int r;
	FILE *fp;
	stat_t f_stat;


	r = -1;

	/* prepare test file */
	unlink("dummy");
	fp = fopen("dummy", "w");
	fwrite("foo", 3, fp);
	fclose(fp);

	/* check stat for test file */
	if(stat("dummy", &f_stat) != 0)
		goto err;

	if(f_stat.type != FT_REG){
		printf(FG_RED "error " RESET_ATTR "type mismatch, having %d\n", f_stat.type);
		goto err;
	}

	if(f_stat.size != 3){
		printf(FG_RED "error " RESET_ATTR "file size mismatch, having %u\n", f_stat.size);
		goto err;
	}

	/* check stat of /dev/tty0 */
	if(stat("/dev/tty0", &f_stat) != 0)
		goto err;

	if(f_stat.type != FT_CHR){
		printf(FG_RED "error " RESET_ATTR "type mismatch, having %d\n", f_stat.type);
		goto err;
	}

	r = 0;


err:
	unlink("dummy");

	return r;

}
