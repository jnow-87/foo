/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <signal.h>
#include <unistd.h>
#include <sys/mutex.h>
#include <test/test.h>


/* local/static prototypes */
static int thread(void *arg);


/* static variables */
static mutex_t mtx = MUTEX_INITIALISER();
static int volatile thread_cnt;


/* local functions */
/**
 *	\brief	test to SIG_KILL
 */
TEST_LONG(thread_kill, "kill"){
	int r;
	process_info_t pinfo;
	tid_t tid;


	r = 0;
	thread_cnt = 0;

	/* prepare */
	r += TEST_INT_EQ(process_info(&pinfo), 0);
	ASSERT_INT_NEQ(tid = thread_create(thread, 0x0), 0);

	while(thread_cnt == 0);

	/* send signal */
	mutex_lock(&mtx);

	r += TEST_INT_EQ(signal_send(SIG_KILL, pinfo.pid, tid), 0);
	thread_cnt = 0;

	mutex_unlock(&mtx);

	r += TEST_INT_EQ(sleep(1000, 0), 0);
	r += TEST_INT_EQ(thread_cnt, 0);

	return -r;
}

static int thread(void *arg){
	while(1){
		mutex_lock(&mtx);
		thread_cnt++;
		mutex_unlock(&mtx);

		sleep(100, 0);
	}

	return 0;
}
