/* brickos header */
#include <sys/escape.h>
#include <testing/testcase.h>

/* host header */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


/* external variables */
extern test_case_t __test_case_hdlr_base[];
extern test_case_t __test_case_hdlr_end[];

extern char* __test_case_desc_base[];
extern char* __test_case_desc_end[];


/* global functions */
int main(int argc, char** argv){
	char** desc;
	int log;
	unsigned int passed, failed;
	test_case_t* hdlr;


	/* init */
	passed = 0;
	failed = 0;

	if(argc != 2){
		printf("usage: %s <log-file>\n", argv[0]);
		return 1;
	}

	log = open(argv[1], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

	if(log < 0){
		printf("unable to open log-file: %s %s\n", argv[1], strerror(errno));
		return 1;
	}

	/* iterate through test cases */
	hdlr = __test_case_hdlr_base;
	desc = __test_case_desc_base;

	while(hdlr != __test_case_hdlr_end && desc != __test_case_desc_end){
		printf("run test case: %s... ", *desc);
		tlog(log, " === test case: %s ===\n\n", *desc);


		if((*hdlr)(log) == 0){
			printf(FG_GREEN "passed\n" RESET_ATTR);
			tlog(log, "\n === test case: %s passed ===\n\n\n", *desc);

			passed++;
		}
		else{
			printf(FG_RED "failed\n" RESET_ATTR);
			tlog(log, "\n === test case: %s failed ===\n\n\n", *desc);

			failed++;
		}

		hdlr++;
		desc++;
	}

	/* summary */
	printf(FG_VIOLETT "\nsummary" RESET_ATTR " (cf. %s)\n"
		   FG_VIOLETT "    total:" RESET_ATTR " %u\n"
		   FG_GREEN "    passed:" RESET_ATTR " %u\n"
		   FG_RED "    failed:" RESET_ATTR " %u\n\n"
		   ,
		   argv[1],
		   passed + failed,
		   passed,
		   failed
	);

	return 0;
}
