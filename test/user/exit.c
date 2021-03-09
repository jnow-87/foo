/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/atomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <test/test.h>


/* macros */
#define NTHREAD		2


/* local/static prototypes */
static int thread(void *arg);


/* static variables */
static int volatile ecode;


/* local functions */
/**
 *	\brief	test process and thread termination
 */
TEST_LONG(exit, "exit"){
	size_t i;


	ecode = 0;

	for(i=0; i<NTHREAD; i++){
		atomic_inc(&ecode, TEST_INT_NEQ(thread_create(thread, 0x0), 0));
	}

	return thread(0x0);
}

static int thread(void *arg){
	thread_info_t info;


	atomic_inc(&ecode, TEST_INT_EQ(thread_info(&info), 0));
	atomic_inc(&ecode, TEST_PTR_EQ(arg, 0x0));

	if(info.tid == 1){
		sleep(2000, 0);

		printf("exit, this should terminate the entire process\n");
		exit(ecode);
	}

	while(1){
		printf("%d still running\n", info.tid);
		sleep(500, 0);
	}

	return -1;
}
