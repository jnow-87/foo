/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <shell/cmds/tests/test.h>


/* macros */
#define NTHREADS		2


/* local/static prototypes */
static int thread(void *arg);
static char args[NTHREADS][5];


/* local functions */
/**
 *	\brief	test to verify user-space signals
 */
static int exec(void){
	unsigned int i;
	tid_t tid;


	/* create threads */
	for(i=0; i<NTHREADS; i++){
		sprintf(args[i], "%d", i + 1);

		tid = thread_create(thread, args[i]);

		if(tid == 0)	ERROR("creating thread \"%s\"\n", strerror(errno));
		else			printf("created thread with id: %u, args: %x \"%s\"\n", tid, args[i], args[i]);
	}

	/* exit */
	thread("main");

	return 0;
}

test("exit", exec, "terminate process and all threads");

static int thread(void *arg){
	thread_info_t info;


	if(thread_info(&info) != 0){
		ERROR("retrieving thread info\n");
		return -1;
	}

	printf("started thread %d with %s\n", info.tid, arg);

	if(info.tid == 1){
		sleep(2000, 0);

		printf("%d exit, this should terminate the entire process\n", info.tid);
		exit(1);
	}


	while(1){
		printf("%d still running\n", info.tid);
		sleep(500, 0);
	}

	return 0;
}
