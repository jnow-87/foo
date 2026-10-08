/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef TESTING_MEMORY_H
#define TESTING_MEMORY_H


#include <sys/types.h>


/* macros */
#define MEMMOCK_ALLOC_FAIL(n, size) ( \
	   (memmock_alloc_fail_callback != 0x0 && memmock_alloc_fail_callback(n, size) == 0) \
	|| (memmock_alloc_fail_in != -1 && memmock_alloc_fail_in-- == 0) \
)


/* prototypes */
void memmock_reset(void);


/* external variables */
extern ssize_t memmock_alloc_fail_in;
extern int (*memmock_alloc_fail_callback)(size_t n, size_t size);


#endif // TESTING_MEMORY_H
