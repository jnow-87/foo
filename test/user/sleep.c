/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <unistd.h>
#include <time.h>
#include <test/test.h>


/* macros */
#define PERIOD_MS	50


/* local functions */
/**
 * \brief	test to verify sleep() returns and the system time
 * 			has advanced according to the sleep interval
 */
TEST(sleep){
	int r = 0;
	uint32_t t0,
			 t1;


	r |= TEST_INT_NEQ(t0 = time_ms(), 0);
	r |= TEST_INT_EQ(sleep(PERIOD_MS, 0), 0);
	r |= TEST_INT_NEQ(t1 = time_ms(), 0);

	r |= TEST_INT_EQ((t1 >= t0 + PERIOD_MS), true);

	return -r;
}
