/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <shell/cmds/test/test.h>


/* local functions */
/**
 * \brief	test to verify stat()
 */
static int exec(void){
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
		ERROR("type mismatch, having %d\n", f_stat.type);
		goto err;
	}

	if(f_stat.size != 3){
		ERROR("file size mismatch, having %u\n", f_stat.size);
		goto err;
	}

	/* check stat of /dev/tty0 */
	if(stat("/dev/tty0", &f_stat) != 0)
		goto err;

	if(f_stat.type != FT_CHR){
		ERROR("type mismatch, having %d\n", f_stat.type);
		goto err;
	}

	r = 0;


err:
	unlink("dummy");

	return r;

}

test("stat", exec, "test the stat() syscall");
