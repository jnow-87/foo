#ifndef SYS_STRING_H
#define SYS_STRING_H


#include <sys/types.h>


/* macros */
#define strcpy(d, s)		(char*)memcpy(d, s, strlen(s) + 1)
#define strncpy(d, s, n)	(char*)memcpy(d, s, n)

#define strncmp(d, s, n)	memcmp(d, s, n)


/* prototypes */
size_t strlen(char const* s);
int strcmp(char const* s0, char const* s1);

void* memcpy(void* dest, void const* src, size_t n);
void* memset(void* p, char c, size_t n);
int memcmp(void const* s0, void const* s1, size_t n);


#endif // SYS_STRING_H