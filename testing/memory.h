/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef TESTING_MEMORY_H
#define TESTING_MEMORY_H


#include <sys/types.h>


/* static variables */
extern size_t test_malloc_fail_at;


/* local/static prototypes */
void test_memory_init(void);
void test_memory_reset(void);


#endif // TESTING_MEMORY_H
