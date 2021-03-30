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
#define SIGNAL	SIG_USR0
#define RETRIES	10


/* local/static prototypes */
static void hdlr(signal_t sig);
static int thread(void *arg);


/* static variables */
static size_t volatile sig_recv;
static signal_t sig_order[NSIG];
static bool volatile finished;
static int volatile errors;
static tid_t tid;


/* local functions */
/**
 *	\brief	test to verify user-space signals
 *			on a target thread
 */
TEST_LONG(sigthread, "sigthread"){
	int r;
	unsigned int i;
	process_info_t pinfo;


	r = 0;
	sig_recv = 0;
	finished = false;
	errors = 0;
	tid = 0;

	memset(sig_order, 0, sizeof(signal_t) * NSIG);

	r += TEST_INT_EQ(process_info(&pinfo), 0);

	for(i=0; i<NSIG; i++){
		r += TEST_PTR_EQ(signal(SIGNAL + i, 0x0), 0x0);
		r += TEST_PTR_EQ(signal(SIGNAL + i, hdlr), hdlr);
	}

	ASSERT_INT_NEQ(tid = thread_create(thread, "foo"), 0);

	for(i=0; i<NSIG; i++){
		r += TEST_INT_EQ(signal_send(SIGNAL + i, pinfo.pid, tid), 0);
		r += TEST_INT_EQ(sleep(300, 0), 0);
	}

	for(i=0; i<NSIG; i++)
		r += TEST_INT_EQ(signal_send(SIGNAL + i, pinfo.pid, tid), 0);

	i = 0;

	while(!finished && i++ < RETRIES){
		r += TEST_INT_EQ(sleep(500, 0), 0);
	}

	r += TEST_INT_NEQ(i, RETRIES);
	r += TEST_INT_EQ(sig_recv, NSIG * 2);
	r += TEST_INT_EQ(errors, 0);

	for(i=0; i<NSIG; i++)
		r += TEST_INT_EQ(sig_order[i], (SIGNAL + i) * 2);

	return -r;
}

#include <stdio.h>
#include <unistd.h>
static void hdlr(signal_t sig){
	char s[32];
	thread_info_t info;


	errors += TEST_INT_EQ(thread_info(&info), 0);
	errors += TEST_INT_EQ(info.tid, tid);

	sig_order[sig_recv % NSIG] += sig;
	sig_recv++;
	snprintf(s, 32, "sig %d %u\n", sig, sig_recv);
	write(1, s, strlen(s));
}

static int thread(void *arg){
	size_t i;


	i = 0;

	while(sig_recv < NSIG * 2 && i++ < RETRIES){
		errors += TEST_INT_EQ(sleep(200, 0), 0);
	}

	printf("thread done %u %u\n", i, sig_recv);
	finished = true;

	return 0;
}
