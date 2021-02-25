/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <test/test.h>


/* macros */
#define NSIG	3
#define SIGNAL	SIG_INT
#define RETRIES	5


/* local/static prototypes */
static void hdlr(signal_t sig);
static int thread(void *arg);


/* static variables */
static size_t volatile sig_recv;
static bool volatile finished;
static int volatile errors;
static tid_t tid;


/* local functions */
/**
 *	\brief	test to verify user-space signals
 *			on a target thread
 */
TEST(sigthread){
	int r;
	unsigned int i;
	process_info_t pinfo;


	r = 0;
	sig_recv = 0;
	finished = false;
	errors = 0;
	tid = 0;

	r += TEST_INT_EQ(process_info(&pinfo), 0);

	r += TEST_PTR_EQ(signal(SIGNAL, 0x0), 0x0);
	r += TEST_PTR_EQ(signal(SIGNAL, hdlr), hdlr);

	r += TEST_INT_NEQ(tid = thread_create(thread, "foo"), 0);
	r += TEST_INT_EQ(sleep(500, 0), 0);

	for(i=0; i<NSIG; i++)
		r += TEST_INT_EQ(signal_send(SIGNAL, pinfo.pid, tid), 0);

	i = 0;

	while(i++ < RETRIES){
		r += TEST_INT_EQ(sleep(500, 0), 0);
	}

	r += TEST_INT_NEQ(i, RETRIES);
	r += TEST_INT_EQ(sig_recv, NSIG);
	r += TEST_INT_EQ(errors, 0);

	return -r;
}

static void hdlr(signal_t sig){
	thread_info_t info;


	errors += TEST_INT_EQ(thread_info(&info), 0);
	errors += TEST_INT_EQ(info.tid, tid);

	sig_recv++;
}

static int thread(void *arg){
	while(sig_recv != NSIG){
		errors += TEST_INT_EQ(sleep(500, 0), 0);
	}

	finished = true;

	return 0;
}
