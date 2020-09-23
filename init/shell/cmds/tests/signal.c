/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/errno.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>
#include <shell/cmds/tests/test.h>


/* macros */
#define NSIGS	3


/* local/static prototypes */
static void hdlr(signal_t sig);
static int thread(void *arg);


/* static variables */
static unsigned int nsigs = 0;
static bool volatile done = false;


/* local functions */
/**
 *	\brief	test to verify user-space signals
 */
static int exec(void){
	int r;
	unsigned int i;
	tid_t tid;
	process_info_t pinfo;


	done = false;
	nsigs = 0;

	/* prepare */
	// get process info
	if(process_info(&pinfo) != 0){
		ERROR("acquiring process info \"%s\"\n", strerror(errno));
		return -1;
	}

	// register signal handler
	if(signal(SIG_INT, hdlr) != hdlr){
		ERROR("registering signal handler \"%s\"\n", strerror(errno));
		return -1;
	}

	// create thread
	tid = thread_create(thread, "foo");

	if(tid == 0){
		ERROR("creating thread \"%s\"\n", strerror(errno));
		return -1;
	}

	sleep(500, 0);

	/* send signals */
	r = 0;

	for(i=0; i<NSIGS; i++)
		r |= signal_send(SIG_INT, pinfo.pid, tid);

	/* wait for thread to terminate */
	while(!done)
		sleep(250, 0);

	/* check result */
	if(r){
		ERROR("sending signal \"%s\"\n", strerror(errno));
		return -1;
	}

	if(nsigs != NSIGS){
		ERROR("%u/%u signals received by thread\n", nsigs, NSIGS);
		return -1;
	}

	return 0;
}

test("signal", exec, "test user-space signals");

static void hdlr(signal_t sig){
	thread_info_t info;


	thread_info(&info);

	printf("%d caught signal %d\n", info.tid, sig);
	nsigs++;
}

static int thread(void *arg){
	unsigned int i;
	thread_info_t info;


	thread_info(&info);

	printf("started thread %d with %s\n", info.tid, arg);

	for(i=0; i<20 && nsigs!=NSIGS; i++)
		sleep(250, 0);

	printf("thread done\n");
	done = true;

	return 0;
}
