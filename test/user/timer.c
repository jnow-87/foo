/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/devicetree.h>
#include <unistd.h>
#include <timer.h>
#include <time.h>
#include <signal.h>
#include <test/test.h>


/* macros */
#define TIMER_PERIOD_MS	(DEVTREE_ARCH_TIMER_CYCLE_TIME_US / 1000)
#define SIG				SIG_USR0
#define NSIG			5


/* local/static prototypes */
static void sig_hdlr(signal_t sig);


/* static variables */
static unsigned int volatile sig_recv = 0;


/* local functions */
/**
 * \brief	test timer_register() and usignals
 */
TEST(timer){
	int r = 0;
	uint32_t t0,
			 t1;


	sig_recv = 0;

	r += TEST_PTR_EQ(signal(SIG, sig_hdlr), sig_hdlr);
	r += TEST_INT_EQ(timer_register(SIG, TIMER_PERIOD_MS * 1000), 0);

	// wait for timer trigger all signals
	sleep(TIMER_PERIOD_MS * NSIG * 2, 0);
	timer_release(SIG);

	// wait for all signals to be delivered
	// currently usignals are not delivered to non-ready threads, e.g. sleeping threads
	// hence a busy waiting loop is needed for the signals to actually be delivered
	t0 = time_ms();
	t1 = t0;

	ASSERT_INT_NEQ(t0, 0);

	while(t1 - t0 < 1000){
		if(sig_recv >= NSIG)
			break;

		t1 = time_ms();
	}

	TEST_LOG("sig_recv: %u\n", sig_recv);
	r += TEST_INT_EQ(sig_recv >= NSIG, true);

	return -r;
}

static void sig_hdlr(signal_t sig){
	sig_recv++;
}
