/*
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <cmd/test/test.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


/* local functions */
/**
 * \brief	test to verify stat()
 */
static int exec(void){
	FILE *fp;
	stat_t f_stat;


	/* prepare test file */
	fp = fopen("dummy", "w");
	fwrite("foo", 3, fp);
	fclose(fp);

	/* check stat for tes file */
	if(stat("dummy", &f_stat) != 0)
		return 1;

	if(f_stat.type != FT_REG){
		printf("type missmatch, having %d\n", f_stat.type);
		return 1;
	}

	if(f_stat.size != 3){
		printf("file size missmatch, haveing %u\n", f_stat.size);
		return 1;
	}

	/* check stat of /dev/tty0 */
	if(stat("/dev/tty0", &f_stat) != 0)
		return 1;

	if(f_stat.type != FT_CHR){
		printf("type missmatch, having %d\n", f_stat.type);
		return 1;
	}

	return 0;
}

test("stat", exec, "test the stat() syscall");
