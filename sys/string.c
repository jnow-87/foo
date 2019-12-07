/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/string.h>
#include <sys/errno.h>


/* global functions */
size_t strlen(char const *s){
	size_t i;


	i = 0;
	while(s[i]) i++;

	return i;
}

int strcmp(char const *s0, char const *s1){
	while(*s0 && *s1){
		if(*s0 < *s1)		return -1;
		else if(*s0 > *s1)	return 1;

		s0++;
		s1++;
	}

	if(*s0 || *s1)
		return -1;

	return 0;
}

void *memcpy(void *dest, void const *src, size_t n){
	size_t i;


	i = n;

	/* TODO optimise */
	if(src < dest && src + n > dest){
		dest += n - 1;
		src += n - 1;

		for(; i>0; i--)
			*(char*)dest-- = *(char*)src--;
	}
	else{
		for(; i>0; i--)
			*(char*)dest++ = *(char*)src++;

		dest -= n;
	}

	return dest;
}

void *memset(void *p, char c, size_t n){
	while(n != 0){
		((char*)p)[--n] = c;
	}

	return p;
}

int memcmp(void const *s0, void const *s1, size_t n){
	size_t i;


	for(i=0; i<n; i++){
		if(((char*)s0)[i] < ((char*)s1)[i])			return -1;
		else if(((char*)s0)[i] > ((char*)s1)[i])	return 1;
	}

	return 0;
}

char const *strerror(errno_t errnum){
	static char const *err_str[] = {
		"Success",
		"Invalid",
		"Out of memory",
		"Resource limitation reached",
		"Input output error",
		"Function not implemented",
		"Resource busy",
		"Resource unavailable",
		"End of resource",
		"Operation not supported",
		"Already connected",
		"No connection",
		"Unkown",
	};
	static char err_unknown[] = "Unkown error 0x....";
	static char err_limit[] = "Error string too short to display errno";


	/* handle unknown errors */
	if(errnum <= E_UNKNOWN)
		return err_str[errnum];

	if(itoa(errnum, 16, err_unknown + 15, 2) == 0x0)
		return err_limit;

	return err_unknown;
}

char *itoa(int v, unsigned int base, char *s, size_t len){
	char d;
	char inv_s[len];
	size_t i;


	/* convert int to (inverted) string */
	i = 0;

	do{
		d = (v % base) % 0xff;
		v /= base;

		if(d < 10)	inv_s[i] = '0' + d;
		else		inv_s[i] = 'a' + d - 10;

		i++;
	}while(v != 0 && i < len);

	/* check if the entire number could be converted */
	if(v != 0)
		return 0x0;

	/* reverse string */
	len = i;

	for(i=0; i<len; i++)
		s[i] = inv_s[len - 1 - i];
	s[i] = 0;

	return s;
}

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
