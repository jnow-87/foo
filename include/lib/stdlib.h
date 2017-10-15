#ifndef LIB_STDLIB_H
#define LIB_STDLIB_H


#include <sys/types.h>


/* prototypes */
void exit(int status);

void *malloc(size_t size);
void free(void *p);


#endif // LIB_STDLIB_H
