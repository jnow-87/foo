/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



/* brickos header */
#include <sys/escape.h>
#include <testing/testcase.h>

/* host header */
#include <stdio.h>
#include <string.h>


/* macros */
#define RESULT_EXT	".results"


/* external variables */
extern test_case_t __start_testcases[];
extern test_case_t __stop_testcases[];


/* global functions */
int main(int argc, char **argv){
	char log_file[strlen(argv[0]) + strlen(RESULT_EXT) + 1];
	int r;
	unsigned int cnt[2] = { 0 };
	test_case_t *tc;


	/* init */
	strcpy(log_file, argv[0]);
	strcpy(log_file + strlen(argv[0]), RESULT_EXT);

	if(test_init(log_file) != 0)
		return 1;

	/* iterate through test cases */
	tc = __start_testcases;

	while(tc != __stop_testcases){
		printf("run test: %s... ", tc->desc);
		test_log(" === test case: %s ===\n\n", tc->desc);

		r = (tc->hdlr)() != 0;

		printf("%s%s\n" RESET_ATTR, ((char *[]){ FG_GREEN, FG_RED }[r]), ((char *[]){ "passed", "failed" }[r]));
		test_log("\n === test case: %s %s ===\n\n\n", tc->desc, ((char *[]){ "passed", "failed" }[r]));
		cnt[r]++;

		tc++;
	}

	test_close();

	/* summary */
	printf(FG_VIOLETT "\nsummary" RESET_ATTR " (cf. %s)\n"
		   FG_VIOLETT "    total:" RESET_ATTR " %u\n"
		   FG_GREEN "    passed:" RESET_ATTR " %u\n"
		   FG_RED "    failed:" RESET_ATTR " %u\n\n"
		   ,
		   log_file,
		   cnt[0] + cnt[1],
		   cnt[0],
		   cnt[1]
	);

	return cnt[1];
}
