/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <unistd.h>
#include <time.h>
#include <test/test.h>


/* local functions */
/**
 * \brief	test to ensure time() returns monotonic increasing values
 */
TEST(time){
	int r = 0;
	uint32_t t0,
			 t1;


	for(size_t i=0; i<10; i++){
		r |= TEST_INT_NEQ(t0 = time_ms(), 0);
		r |= TEST_INT_EQ(sleep(50, 0), 0);
		r |= TEST_INT_NEQ(t1 = time_ms(), 0);

		r |= TEST_INT_EQ(t1 >= t0, true);
	}

	return -r;
}
