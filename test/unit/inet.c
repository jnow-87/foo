/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/inet.h>
#include <sys/string.h>
#include <test/test.h>


/* local functions */
TEST(inet){
	int n = 0;
	char s[16];
	inet_addr_t addr;


	addr = inet_addr("192.168.0.1");
	n += TEST_INT_EQ(addr & 0xff, 1);
	n += TEST_INT_EQ((addr >> 8) & 0xff, 0);
	n += TEST_INT_EQ((addr >> 16) & 0xff, 168);
	n += TEST_INT_EQ((addr >> 24) & 0xff, 192);
	n += TEST_PTR_EQ(inet_ntoa_r(addr, s, 16), s);
	n += TEST_STR_EQ(s, "192.168.0.1");

	addr = inet_addr("0.0.0.0");
	n += TEST_STR_EQ(inet_ntoa(addr), "0.0.0.0");

	return -n;
}
