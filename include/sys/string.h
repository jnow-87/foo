/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef _STRING_H		// include guard against host header
#ifndef SYS_STRING_H
#define SYS_STRING_H


#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/errno.h>


/* macros */
#define ESC_RESOLVE_NONE	127

#define strncpy(d, s, n)	(char*)memcpy(d, s, n)
#define strncmp(d, s, n)	memcmp(d, s, n)

#define callbacks_set(p, type) \
	(memnscan(p, (int (*[])()){ 0x0 }, sizeof(type) / sizeof(int (*)()), sizeof(int (*)())) == 0x0)


/* prototypes */
char *strcpy(char *dest, char const *src);
size_t strlen(char const *s);
int strcmp(char const *s0, char const *s1);
size_t strcnt(char const *s, char c);
char *strcat(char *dest, char const *src);
char *strncat(char *dest, char const *src, size_t n);
char const *strchr(char const *s, int c);
char const *strrchr(char const *s, int c);
char const *strpchr(char const *s, int (*cmp)(int));

bool isoneof(char c, char const *s);

void *memcpy(void *dest, void const *src, size_t n) __used;
void *memset(void *p, char c, size_t n) __used;
int memcmp(void const *s0, void const *s1, size_t n) __used;
void *memscan(void *addr, char c, size_t n);
void *memnscan(void *s0, void *s1, size_t nmemb, size_t size);

char const *strerror(errno_t errno);

char *itoa(int v, unsigned int base, char *s, size_t n);
int atoi(char const *s);
long int strtol(char const *p, char **endp, int base);

char *strupr(char const *s);
char *strupr_r(char const *s, char *buf, size_t n);

char *strcident(char const *s);
char *strcident_r(char const *s, char *buf, size_t n);

void strdeesc(char *s, size_t n, int (*resolve)(int));


#endif // SYS_STRING_H
#endif // _STRING_H
