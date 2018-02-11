#ifndef LIB_STDLIB_H
#define LIB_STDLIB_H


#include <config/config.h>
#include <sys/compiler.h>
#include <sys/types.h>


/* macros */
#ifndef CONFIG_SC_DYNMEM

#define malloc(size)	(void*)CALL_DISABLED(malloc, CONFIG_SC_DYNMEM)
#define free(addr)		CALL_DISABLED(free, CONFIG_SC_DYNMEM)

#endif // CONFIG_SC_DYNMEM


/* prototypes */
#ifdef CONFIG_SC_DYNMEM

void *malloc(size_t size);
void free(void *addr);

#endif // CONFIG_SC_DYNMEM

void exit(int status);


#endif // LIB_STDLIB_H
