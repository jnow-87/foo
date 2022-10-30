/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/types.h>
#include <sys/limits.h>
#include <sys/string.h>
#include <sys/errno.h>


/* local/static prototypes */
static char const *strchr_base(char const *s, int c, char const *end);


/* global functions */
char *strcpy(char *dest, char const *src){
	return memcpy(dest, src, strlen(src) + 1);
}

size_t strlen(char const *s){
	size_t i = 0;


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

size_t strcnt(char const *s, char c){
	size_t n = 0;


	while(*s){
		if(*(s++) == c)
			n++;
	}

	return n;
}

char *strcat(char *dest, char const *src){
	size_t dlen;


	dlen = strlen(dest);
	strcpy(dest + dlen, src);

	return dest;
}

char *strncat(char *dest, char const *src, size_t n){
	size_t dlen;


	dlen = strlen(dest);
	strncpy(dest + dlen, src, n);
	dest[dlen + n] = 0;

	return dest;
}

char const *strchr(char const *s, int c){
	return strchr_base(s, c, 0x0);
}

char const *strrchr(char const *s, int c){
	return strchr_base(s + strlen(s), c, s);
}

char const *strpchr(char const *s, int (*cmp)(int)){
	while(*s != 0 && !cmp(*s))
		s++;

	return s;
}

bool isoneof(char c, char const *s){
	if(s == 0x0)
		return false;

	for(; *s; s++){
		if(c == *s)
			return true;
	}

	return false;
}

void *memcpy(void *dest, void const *src, size_t n){
	size_t i = n;


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
	for(size_t i=0; i<n; i++){
		if(((char*)s0)[i] < ((char*)s1)[i])			return -1;
		else if(((char*)s0)[i] > ((char*)s1)[i])	return 1;
	}

	return 0;
}

void *memscan(void *addr, char c, size_t n){
	return memnscan(addr, &c, n, 1);
}

void *memnscan(void *s0, void *s1, size_t nmemb, size_t size){
	for(size_t i=0; i<nmemb; i++){
		if(memcmp(s0 + i * size, s1, size) == 0)
			return s0 + i * size;
	}

	return 0x0;
}

char const *strerror(errno_t errnum){
	static char const *err_str[] = {
		"Success",
		"Invalid",
		"Out of memory",
		"Resource limitation reached",
		"Input output",
		"Function not implemented",
		"Resource busy",
		"Resource unavailable",
		"Try again",
		"End of resource",
		"Operation not supported",
		"Already connected",
		"No connection",
		"Unknown",
		"Invalid errno"
	};


	if(errnum > E_UNKNOWN)
		errnum = E_UNKNOWN + 1;

	return err_str[errnum];
}

char *itoa(int v, unsigned int base, char *s, size_t n){
	size_t i = 0;
	char d;
	char inv_s[n];


	/* convert int to (inverted) string */
	do{
		d = (v % base) % 0xff;
		v /= base;

		if(d < 10)	inv_s[i] = '0' + d;
		else		inv_s[i] = 'a' + d - 10;

		i++;
	}while(v != 0 && i < n);

	/* check if the entire number could be converted */
	if(v != 0)
		return 0x0;

	/* reverse string */
	n = i;

	for(i=0; i<n; i++)
		s[i] = inv_s[n - 1 - i];
	s[i] = 0;

	return s;
}

int atoi(char const *s){
	return strtol(s, 0x0, 10);
}

long int strtol(char const *p, char **endp, int base){
	int sign = 1;
	long int r = 0;
	long int x;


	/* check optional sign */
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

char *strupr(char const *s){
	static char _s[NAME_MAX + 1];


	return strupr_r(s, _s, NAME_MAX + 1);
}

char *strupr_r(char const *s, char *buf, size_t n){
	size_t i;


	for(i=0; s[i]!=0 && i<n; i++){
		if(s[i] >= 'a' && s[i] <= 'z')	buf[i] = s[i] - 32;
		else							buf[i] = s[i];
	}

	buf[i] = 0;

	return buf;
}

char *strcident(char const *s){
	static char _s[NAME_MAX + 1];


	return strcident_r(s, _s, NAME_MAX + 1);
}

char *strcident_r(char const *s, char *buf, size_t n){
	size_t i;


	for(i=0; s[i]!=0 && i<n; i++){
		switch(s[i]){
		case '-':	buf[i] = '_'; break;
		default:	buf[i] = s[i]; break;
		}
	}

	buf[i] = 0;

	return buf;
}

void strdeesc(char *s, size_t n, int (*resolve)(int)){
	char *e = s + n;


	for(char *c=s; c+1<e; c++){
		if(*c != '\\')
			continue;

		*c = resolve(c[1]);

		if(*c != ESC_RESOLVE_NONE){
			memcpy(c + 1, c + 2, e - c - 1);
			*(--e) = 0;
		}
		else
			*c = '\\';
	}
}


/* local functions */
static char const *strchr_base(char const *s, int c, char const *end){
	int dir = end ? -1 : 1;


	while(1){
		if(*s == c)
			return s;

		if(s == end || (!end && *s == 0))
			return 0x0;

		s += dir;
	}
}
