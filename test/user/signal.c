/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <signal.h>
#include <unistd.h>
#include <test/test.h>


/* macros */
#define NSIG	3
#define SIGNAL	SIG_INT


/* local/static prototypes */
static void hdlr(signal_t sig);


/* static variables */
static unsigned int volatile sig_recv;


/* local functions */
/**
 *	\brief	test to verify user-space signals
 */
TEST(sigself){
	int r = 0;
	process_info_t pinfo;
	thread_info_t tinfo;


	sig_recv = 0;

	r += TEST_INT_EQ(process_info(&pinfo), 0);
	r += TEST_INT_EQ(thread_info(&tinfo), 0);
	r += TEST_PTR_EQ(signal(SIGNAL, 0x0), 0x0);
	r += TEST_PTR_EQ(signal(SIGNAL, hdlr), hdlr);

	for(size_t i=0; i<NSIG; i++)
		r += TEST_INT_EQ(signal_send(SIGNAL, pinfo.pid, tinfo.tid), 0);

	r += TEST_INT_EQ(sig_recv, NSIG);

	return -r;
}

static void hdlr(signal_t sig){
	sig_recv++;
}
