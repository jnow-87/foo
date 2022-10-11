/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/math.h>
#include <sys/string.h>
#include <sys/stream.h>
#include <sys/register.h>
#include <sys/inet.h>


/* global functions */
inet_addr_t inet_addr(char *s){
	uint8_t j = 0,
			len = strlen(s);
	inet_addr_t iaddr = 0;
	char addr[len + 1];


	strcpy(addr, s);

	for(int8_t i=0; i<=len; i++){
		if(addr[i] == '.' || addr[i] == 0){
			addr[i] = 0;
			iaddr = (iaddr << 8) | atoi(addr + j);

			j = i + 1;
		}
	}

	return iaddr;
}

char *inet_ntoa(inet_addr_t addr){
	static char ip[16] = { 0 };


	return inet_ntoa_r(addr, ip, 16);
}

char *inet_ntoa_r(inet_addr_t addr, char *s, size_t n){
	snprintf(s, n, "%hhu.%hhu.%hhu.%hhu",
		(uint8_t)bits(addr, 24, 0xff),
		(uint8_t)bits(addr, 16, 0xff),
		(uint8_t)bits(addr, 8, 0xff),
		(uint8_t)bits(addr, 0, 0xff)
	);

	return s;
}
