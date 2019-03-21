/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */


#ifndef _STRING_H		// include guard against host header
#ifndef SYS_STRING_H
#define SYS_STRING_H


#include <sys/types.h>
#include <sys/errno.h>


/* macros */
#define strcpy(d, s)		(char*)memcpy(d, s, strlen(s) + 1)
#define strncpy(d, s, n)	(char*)memcpy(d, s, n)

#define strncmp(d, s, n)	memcmp(d, s, n)


/* prototypes */
size_t strlen(char const *s);
int strcmp(char const *s0, char const *s1);

void *memcpy(void *dest, void const *src, size_t n);
void *memset(void *p, char c, size_t n);
int memcmp(void const *s0, void const *s1, size_t n);

char const *strerror(errno_t errno);

char *itoa(int v, unsigned int base, char *s, size_t len);

#endif // SYS_STRING_H
#endif // _STRING_H
