/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/types.h>


/* global functions */
uint8_t bits_set(unsigned int val){
	uint8_t n = 0;


	for(uint8_t i=0; i<32; i++){
		n += (val & 0x1);
		val >>= 1;
	}

	return n;
}

int8_t bits_highest(unsigned int val){
	for(uint8_t i=32; i>0; i--){
		if(val & (0x1 << (i - 1)))
			return i - 1;
	}

	return -1;
}
