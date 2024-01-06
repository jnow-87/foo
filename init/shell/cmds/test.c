/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/escape.h>
#include <test/test.h>
#include <shell/shell.h>
#include <shell/cmd.h>


/* macros */
#define ARGS	"<test-nr>"
#define OPTS \
	"-l <log-file>", "log-file name", \
	"-h", "print this help message"

#define NUM_INT_TESTS		((size_t)(__stop_tests_user_interactive - __start_tests_user_interactive))
#define NUM_NONINT_TESTS	((size_t)(__stop_tests_user_noninteractive - __start_tests_user_noninteractive))
#define NUM_UNIT_TESTS		((size_t)(__stop_tests_unit - __start_tests_unit))


/* local/static prototypes */
static int run(test_t *tests, size_t n, bool summary, char const *log_name);
static int help(char const *cmd_name, char const *error);


/* external variables */
extern test_t __start_tests_user_interactive[],
			  __stop_tests_user_interactive[],
			  __start_tests_user_noninteractive[],
			  __stop_tests_user_noninteractive[];

#ifdef CONFIG_INIT_TEST_UNIT
extern test_t __start_tests_unit[],
			  __stop_tests_unit[];
#endif // CONFIG_INIT_TEST_UNIT


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
	char opt;
	size_t n;
	char const *log_name;


	/* parse options */
	log_name = 0x0;

	while((opt = getopt(argc, argv, "l:h")) != -1){
		switch(opt){
		case 'l':	log_name = optarg; break;
		case 'h':	return help(argv[0], 0x0);
		default:	return help(argv[0], "");
		}
	}

	if(optind >= argc)
		return help(argv[0], "missing arguments");

	n = (size_t)atoi(argv[optind]);

	/* run tests */
	if(n < NUM_INT_TESTS)
		return run(__start_tests_user_interactive + n, 1, false, log_name);

	if(n == NUM_INT_TESTS)
		return run(__start_tests_user_noninteractive, NUM_NONINT_TESTS, true, log_name);

#ifdef CONFIG_INIT_TEST_UNIT
	if(n == NUM_INT_TESTS + 1)
		return run(__start_tests_unit, NUM_UNIT_TESTS, true, log_name);
#endif // CONFIG_INIT_TEST_UNIT

	return -ERROR("invalid test number");
}

command("test", exec);

static int run(test_t *tests, size_t num, bool summary, char const *log_name){
	int r;
	unsigned int results[2] = { 0 };


	if(log_name)
		log = fopen(log_name, "w");

	for(size_t i=0; i<num; i++){
		if(summary)
			printf(" %s... ", tests[i].name);

		r = (tests[i].call() != 0);
		results[r]++;

		printf("%s%s" RESET_ATTR "\n",
			((char const *[]){ FG_GREEN, FG_RED }[r]),
			((char const *[]){ "passed", "failed" }[r])
		);
	}

	if(log_name)
		fclose(log);

	log = 0x0;

	if(summary){
		printf(
			FG("\nsummary", PURPLE) "\n"
			FG("    total", PURPLE) ": %u\n"
			FG("    passed", GREEN) ": %u\n"
			FG("    failed", RED) ": %u\n"
			,
			results[0] + results[1],
			results[0],
			results[1]
		);
	}

	return -results[1];
}

static int help(char const *cmd_name, char const *error){
	size_t i = 0;
	test_t *test;


	CMD_HELP(cmd_name, error);

	printf("\ntests:\n\t%6.6s    %15.15s    %s\n", "Number", "Name", "Description");

	test_for_each_type(test, user_interactive)
		printf("\t%6d    %15.15s    %s\n", i++, test->name, test->descr);

	printf("\t%6d    %15.15s    %s\n", i++, "non-int", "non-interactive tests");

#ifdef CONFIG_INIT_TEST_UNIT
	printf("\t%6d    %15.15s    %s\n", i++, "unit", "unit tests");
#endif // CONFIG_INIT_TEST_UNIT

	return (error != 0x0) ? 1 : 0;
}
