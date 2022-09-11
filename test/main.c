/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifdef BUILD_HOST
# include <stdio.h>
# include <stdarg.h>
# include <errno.h>
# include <string.h>
# include <fcntl.h>
#else
# include <sys/stdarg.h>
# include <sys/errno.h>
# include <sys/string.h>
# include <sys/fcntl.h>
# include <lib/stdio.h>
#endif // BUILD_HOST

#include <sys/compiler.h>
#include <sys/escape.h>
#include <test/test.h>


/* macros */
#define RESULT_EXT	".log"


/* local/static prototypes */
static int test_log_open(char const *name);
static void test_log_close(void);


/* static variables */
static FILE *log = 0x0;


/* global functions */
int main(int argc, char **argv){
	char log_file[strlen(argv[0]) + strlen(RESULT_EXT) + 1];
	int r;
	unsigned int results[2] = { 0 };
	test_t *test;


	/* init */
	strcpy(log_file, argv[0]);
	strcpy(log_file + strlen(argv[0]), RESULT_EXT);

	if(test_log_open(log_file) != 0)
		return 1;

	/* iterate through test cases */
	printf("execute %s tests\n", STR(TESTS));

	test_for_each_type(test, TESTS){
		printf(" %s... ", test->name);

		r = (test->call() != 0);
		results[r]++;

		printf("%s%s" RESET_ATTR "\n",
			((char const *[]){ FG_GREEN, FG_RED }[r]),
			((char const *[]){ "passed", "failed" }[r])
		);
	}

	test_log_close();

	/* summary */
	printf(
		FG_VIOLETT "\nsummary" RESET_ATTR " (cf. %s)\n"
		FG_VIOLETT "    total:" RESET_ATTR " %u\n"
		FG_GREEN "    passed:" RESET_ATTR " %u\n"
		FG_RED "    failed:" RESET_ATTR " %u\n"
		,
		log_file,
		results[0] + results[1],
		results[0],
		results[1]
	);

	return results[1];
}

void test_log(char const *fmt, ...){
	va_list lst;


	va_start(lst, fmt);

#ifdef BUILD_HOST
	vdprintf(fileno(log), fmt, lst);
#else
	vfprintf(log, fmt, lst);
#endif // BUILD_HOST

	va_end(lst);
}


/* local functions */
static int test_log_open(char const *name){
	log = fopen(name, "w");

	stderr = log;

	if(log == 0x0)
		fprintf(stderr, "open log-file \"%s\" failed with %s\n", name, strerror(errno));

	return -(log == 0x0);
}

static void test_log_close(void){
	fclose(log);
}
