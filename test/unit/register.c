/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/register.h>
#include <test/test.h>


/* local functions */
TEST(bits_get){
	int r = 0;


	r += TEST_INT_EQ(bits_get(0x80, 7, 0x1), 0x1);
	r += TEST_INT_EQ(bits_get(0x80, 6, 0x1), 0x0);
	r += TEST_INT_EQ(bits_get(0x80, 8, 0x1), 0x0);
	r += TEST_INT_EQ(bits_get(0x85, 2, 0x21), 0x21);
	r += TEST_INT_EQ(bits_get(0x81, 2, 0x21), 0x20);
	r += TEST_INT_EQ(bits_get(0x81, 3, 0x21), 0x0);

	return -r;
}

TEST(bits_set){
	int r = 0;


	r += TEST_INT_EQ(bits_set(0x0, 0x5), 0x5);
	r += TEST_INT_EQ(bits_set(0x1, 0x5), 0x5);
	r += TEST_INT_EQ(bits_set(0x8, 0x5), 0xd);
	r += TEST_INT_EQ(bits_set(0xa, 0x0), 0xa);

	return -r;
}

TEST(bits_count){
	int r = 0;


	r += TEST_INT_EQ(bits_count(0x00), 0);
	r += TEST_INT_EQ(bits_count(0x01), 1);
	r += TEST_INT_EQ(bits_count(0x02), 1);
	r += TEST_INT_EQ(bits_count(0x07), 3);
	r += TEST_INT_EQ(bits_count(0x08), 1);
	r += TEST_INT_EQ(bits_count(0x80), 1);
	r += TEST_INT_EQ(bits_count(0x81), 2);
	r += TEST_INT_EQ(bits_count(0x91), 3);

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
