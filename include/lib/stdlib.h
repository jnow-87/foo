/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef LIB_STDLIB_H
#define LIB_STDLIB_H


#include <config/config.h>
#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/string.h>


/* prototypes */
void *malloc(size_t size);
void *calloc(size_t n, size_t size);
void free(void *addr);

void exit(int status);
void _exit(int status, bool kill_sibl);


/* disabled-call macros */
#ifndef CONFIG_SC_DYNMEM

#define malloc(size)	(void*)CALL_DISABLED(malloc, CONFIG_SC_DYNMEM)
#define free(addr)		CALL_DISABLED(free, CONFIG_SC_DYNMEM)

#endif // CONFIG_SC_DYNMEM


#endif // LIB_STDLIB_H
