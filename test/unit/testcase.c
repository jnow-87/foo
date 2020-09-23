/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>


/* static variables */
static int test_logfd = -1;


/* global functions */
int test_init(char const *log_name){
	test_logfd = open(log_name, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

	if(test_logfd < 0){
		fprintf(stderr, "open log-file \"%s\" failed \"%s\"\n", log_name, strerror(errno));
		return -1;
	}

	return 0;
}

void test_close(void){
	close(test_logfd);
}

void test_log(char const *fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	vdprintf(test_logfd, fmt, lst);
	va_end(lst);
}
