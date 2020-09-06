/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/inet.h>
#include <sys/string.h>
#include <testing/testcase.h>


/* local functions */
static int tc_inet(int log){
	int n;
	char s[16];
	inet_addr_t addr;


	n = 0;

	addr = inet_addr("192.168.0.1");
	n += check_int(log, addr & 0xff, 1);
	n += check_int(log, (addr >> 8) & 0xff, 0);
	n += check_int(log, (addr >> 16) & 0xff, 168);
	n += check_int(log, (addr >> 24) & 0xff, 192);
	n += check_ptr(log, inet_ntoa_r(addr, s, 16), s);
	n += check_str(log, , s, "192.168.0.1");

	addr = inet_addr("0.0.0.0");
	n += check_str(log, , inet_ntoa(addr), "0.0.0.0");

	return -n;
}

test_case(tc_inet, "inet");
