/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stdarg.h>
#include <sys/escape.h>
#include <test/test.h>
#include <shell/cmd.h>


/* macros */
#define NUM_INT_TESTS		((size_t)(__stop_tests_user_interactive - __start_tests_user_interactive))
#define NUM_NONINT_TESTS	((size_t)(__stop_tests_user_noninteractive - __start_tests_user_noninteractive))


/* local/static prototypes */
static int run(test_t *tests, size_t n, bool summary, char const *log_name);
static int help(char const *cmd_name, char const *msg, ...);


/* external variables */
extern test_t __start_tests_user_interactive[],
			  __stop_tests_user_interactive[],
			  __start_tests_user_noninteractive[],
			  __stop_tests_user_noninteractive[];


/* static variables */
static FILE *log = 0x0;


/* global functions */
void test_log(char const *fmt, ...){
	va_list lst;


	if(log == 0x0)
		return;

	va_start(lst, fmt);
	vfprintf(log, fmt, lst);
	va_end(lst);
}


/* local functions */
static int exec(int argc, char **argv){
	int i;
	size_t n;
	char const *log_name;


	/* parse options */
	log_name = 0x0;

	for(i=1; i<argc && argv[i][0]=='-'; i++){
		switch(argv[i][1]){
		case 'l':
			if(i + 1 >= argc)
				return help(argv[0], "missing argument to option '%s'", argv[i]);

			log_name = argv[++i];
			break;

		default:
			return help(argv[0], "invalid option '%s'\n", argv[i]);
		}
	}

	if(i >= argc)
		return help(argv[0], "missing arguments");

	n = (size_t)atoi(argv[i]);

	/* run tests */
	if(n < NUM_INT_TESTS)
		return run(__start_tests_user_interactive + n, 1, false, log_name);

	if(n == NUM_INT_TESTS)
		return run(__start_tests_user_noninteractive, NUM_NONINT_TESTS, true, log_name);

	printf("Invalid test number\n\n");

	return 1;
}

command("test", exec);

static int run(test_t *tests, size_t num, bool summary, char const *log_name){
	int r;
	size_t i;
	unsigned int results[2] = { 0 };


	if(log_name)
		log = fopen(log_name, "w");

	for(i=0; i<num; i++){
		if(summary)
			printf(" %s... ", tests[i].name);

		r = (tests[i].call() != 0);
		results[r]++;

		printf("%s%s" RESET_ATTR "\n",
			((char *[]){ FG_GREEN, FG_RED }[r]),
			((char *[]){ "passed", "failed" }[r])
		);
	}

	if(log_name)
		fclose(log);

	log = 0x0;

	if(summary){
		printf(
			FG_VIOLETT "\nsummary" RESET_ATTR "\n"
			FG_VIOLETT "    total:" RESET_ATTR " %u\n"
			FG_GREEN "    passed:" RESET_ATTR " %u\n"
			FG_RED "    failed:" RESET_ATTR " %u\n"
			,
			results[0] + results[1],
			results[0],
			results[1]
		);
	}

	return -results[1];
}

static int help(char const *cmd_name, char const *msg, ...){
	size_t i;
	test_t *test;
	va_list lst;


	if(msg){
		va_start(lst, msg);
		vfprintf(stderr, msg, lst);
		va_end(lst);
		fputs("\n", stderr);
	}

	fprintf(stderr,
		"usage: %s [options] <test-nr>\n"
		"\n"
		"options:\n"
		"%15.15s    %s\n"
		"\n"
		, cmd_name
		, "-l", "log-file name"
	);

	printf("\t%6.6s    %15.15s    %s\n", "Number", "Name", "Description");

	i = 0;

	test_for_each_type(test, user_interactive)
		printf("\t%6d    %15.15s    %s\n", i++, test->name, test->descr);

	printf("\t%6d    %15.15s    %s\n", i++, "non-int", "non-interactive tests");

	return 0;
}
