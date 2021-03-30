/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/atomic.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <test/test.h>


/* macros */
#define NTHREAD	1
#define RETRIES	5


/* local/static prototypes */
static int thread(void *arg);


/* static variables */
static int volatile errors;
static int volatile finished;
static tid_t volatile tids_ref[NTHREAD],
					  tids_info[NTHREAD];


/* local functions */
/**
 * \brief	test to verify thread_create() works and passes the correct arguments.
 * 			also sleep() and mutex functions are tested.
 */
TEST(thread_create){
	int r;
	size_t i;
	tid_t tid;


	r = 0;
	errors = 0;
	finished = 0;

	/* create threads */
	for(i=0; i<NTHREAD; i++){
		ASSERT_INT_NEQ(tid = thread_create(thread, (void*)(tids_info + i)), 0);
		tids_ref[i] = tid;
	}

	/* sync threads */
	i = 0;

	while(finished != NTHREAD && i++ < RETRIES){
		msleep(500);
	}

	/* check tids */
	r += TEST_INT_NEQ(i, RETRIES);
	r += TEST_INT_EQ(errors, 0);

	for(i=0; i<NTHREAD; i++){
		r += TEST_INT_EQ(tids_info[i], tids_ref[i]);
	}

	return -r;
}

static int thread(void *arg){
	int r;
	thread_info_t info;
	unsigned int *tid;


	tid = (unsigned int*)arg;

	r = 0;
	r += TEST_INT_EQ(thread_info(&info), 0);

	*tid = info.tid;

	atomic_inc(&errors, r);
	atomic_inc(&finished, 1);

	return -r;
}
