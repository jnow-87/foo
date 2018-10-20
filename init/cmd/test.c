#include <sys/escape.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmd/cmd.h>
#include <cmd/test/test.h>


/* macros */
#define NUM_TESTS	((__tests_end - __tests_start))


/* external variables */
extern test_t __tests_start[],
			  __tests_end[];


/* local functions */
static int exec(int argc, char **argv){
	int i,
		r;
	test_t *test;


	if(argc < 2)
		goto help;

	/* get test number */
	i = atoi(argv[1]) - 1;

	if(i < 0 || i >= NUM_TESTS){
		printf("Invalid test number\n\n");
		return 1;
	}

	/* execute test */
	r = __tests_start[i].exec();
	printf("\ntest %s\n", (r == 0 ? FG_GREEN "succeed" RESET_ATTR : FG_RED "failed" RESET_ATTR));

	return 0;


help:
	printf("usage: %s <test-nr>\n", argv[0]);
	printf("\t%6.6s    %15.15s    %s\n", "Number", "Name", "Description");

	for(test=__tests_start, i=1; test!=__tests_end; test++, i++)
		printf("\t%6d    %15.15s    %s\n", i, test->name, test->descr);

	return 1;
}

command("test", exec);
