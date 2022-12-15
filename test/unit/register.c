/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/register.h>
#include <test/test.h>


/* local functions */
TEST(bits_set){
	int r = 0;


	r += TEST_INT_EQ(bits_set(0x00), 0);
	r += TEST_INT_EQ(bits_set(0x01), 1);
	r += TEST_INT_EQ(bits_set(0x02), 1);
	r += TEST_INT_EQ(bits_set(0x07), 3);
	r += TEST_INT_EQ(bits_set(0x08), 1);
	r += TEST_INT_EQ(bits_set(0x80), 1);
	r += TEST_INT_EQ(bits_set(0x81), 2);
	r += TEST_INT_EQ(bits_set(0x91), 3);

	return -r;
}

TEST(bits_highest){
	int r = 0;


	r += TEST_INT_EQ(bits_highest(0x00), -1);
	r += TEST_INT_EQ(bits_highest(0x01), 0);
	r += TEST_INT_EQ(bits_highest(0x02), 1);
	r += TEST_INT_EQ(bits_highest(0x07), 2);
	r += TEST_INT_EQ(bits_highest(0x08), 3);
	r += TEST_INT_EQ(bits_highest(0x80), 7);
	r += TEST_INT_EQ(bits_highest(0x81), 7);
	r += TEST_INT_EQ(bits_highest(0x91), 7);

	return -r;
}
