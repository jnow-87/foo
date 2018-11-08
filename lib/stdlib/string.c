/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <lib/string.h>
#include <lib/stdlib.h>


/* global functions */
int atoi(char const *s){
	return strtol(s, 0x0, 10);
}

long int strtol(char const *p, char **endp, int base){
	int sign;
	long int r,
			 x;


	/* check optional sign */
	sign = 1;

	if(*p == '-'){
		sign = -1;
		p++;
	}
	else if(*p == '+')
		p++;

	/* check optional prefix to determine the base */
	if(strncmp(p, "0x", 2) == 0 && (base == 0 || base == 16)){
		base = 16;
		p += 2;
	}

	if(base == 0){
		if(*p == '0'){
			base = 8;
			p++;
		}
		else
			base = 10;
	}

	/* parse number */
	r = 0;

	while(*p != 0){
		if(*p >= '0' && *p <= '9')			x = *p - '0';
		else if(*p >= 'a' && *p <= 'f')		x = *p - 'a' + 10;
		else if(*p >= 'A' && *p <= 'F')		x = *p - 'A' + 10;
		else								break;

		if(x >= base)
			break;

		r = r * base + x;
		p++;
	}

	/* return result */
	if(endp != 0x0)
		*endp = (char*)p;

	return r * sign;
}
