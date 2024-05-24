/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#include <sys/math.h>
#include <test/test.h>


/* local/static prototypes */
static int test_align(unsigned int x, unsigned int base);


/* local functions */
TEST(alignp2){
	int r = 0;


	r |= test_align(1, 1);
	r |= test_align(1, 2);
	r |= test_align(2, 2);
	r |= test_align(3, 2);
	r |= test_align(1, 4);
	r |= test_align(20, 16);

	return -r;
}

static int test_align(unsigned int x, unsigned int base){
	x = ALIGNP2(x, base);

	return TEST_INT_EQ(x % base, 0);
}
