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
#include <shell/cmd/test/test.h>


/* local/static prototypes */
static int thread(void *arg);


/* local functions */
/**
 *	\brief	test to verify user-space signals
 */
static int exec(void){
	process_info_t pinfo;
	tid_t tid;


	/* prepare */
	// get process info
	if(process_info(&pinfo) != 0){
		ERROR("acquiring process info \"%s\"\n", strerror(errno));
		return -1;
	}

	// create thread
	tid = thread_create(thread, "foo");

	if(tid == 0){
		ERROR("creating thread \"%s\"\n", strerror(errno));
		return -1;
	}

	sleep(2000, 0);

	/* send signal */
	if(signal_send(SIG_KILL, pinfo.pid, tid) != 0){
		ERROR("sending signal \"%s\"\n", strerror(errno));
		return -1;
	}

	return 0;
}

test("thread kill", exec, "kill a thread through SIG_KILL");

static int thread(void *arg){
	thread_info_t info;


	thread_info(&info);

	printf("started thread %d with %s\n", info.tid, arg);

	while(1){
		printf("%d still running\n", info.tid);
		sleep(500, 0);
	}

	return 0;
}
