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
	while(1){
		((char*)p)[--n] = c;

		if(n == 0)
			break;
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
