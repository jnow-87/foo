/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/escape.h>
#include <sys/mutex.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sched.h>
#include <shell/cmd/test/test.h>


/* macros */
#define NTHREADS		3
#define NTHREAD_ITER	3
#define TIMEOUT			5
#define PERIOD_MS		500

#define ATOMIC_INC(var, mtx){ \
	mutex_lock(&mtx); \
	var++; \
	mutex_unlock(&mtx); \
}


/* local/static prototypes */
static int thread(void *arg);


/* static variables */
static mutex_t m = MUTEX_INITIALISER();
static int volatile r = 0;
static int volatile finished = 0;
static char args[NTHREADS][5];


/* local functions */
/**
 * \brief	test to verify thread_create() works and passes the correct arguments.
 * 			also sleep() and mutex functions are tested.
 */
static int exec(void){
	size_t i;
	unsigned int to;
	tid_t tid;


	/* create threads */
	for(i=0; i<NTHREADS; i++){
		sprintf(args[i], "%d", i + 1);

		tid = thread_create(thread, args[i]);

		if(tid == 0){
			ERROR("creating thread \"%s\"\n", strerror(errno));
			return -1;
		}

		printf("created thread with id: %u, args: %x \"%s\"\n", tid, args[i], args[i]);
	}

	/* wait for threads to finish */
	to = 0;

	while(finished != 3){
		msleep(PERIOD_MS * NTHREAD_ITER);

		if(++to >= TIMEOUT){
			ERROR("timeout detected, threads did not return in time\n");
			return -1;
		}
	}

	return r;
}

test("thread-create", exec, "test thread creation and sleep");


static int thread(void *arg){
	int i;
	char ref[5];
	thread_info_t tinfo;


	thread_info(&tinfo);
	printf("thread %u started with args %x \"%s\"\n", tinfo.tid, arg, arg);

	/* check argument */
	sprintf(ref, "%u", tinfo.tid);

	if(strcmp(arg, ref) != 0){
		ERROR("argument doesn't match thread id (%s != %s)\n", arg, ref);
		ATOMIC_INC(r, m);

		goto end;
	}

	/* test sleep */
	for(i=NTHREAD_ITER; i>=0; i--){
		printf("thread %u %d\n", tinfo.tid, i);
		msleep(PERIOD_MS);
	}


end:
	ATOMIC_INC(finished, m);

	return 20 + tinfo.tid;
}
