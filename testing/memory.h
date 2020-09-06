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
extern size_t tmalloc_fail_at;


/* local/static prototypes */
void tmemory_init(void);
void tmemory_reset(void);

void *tmalloc(size_t size);
void *tcalloc(size_t n, size_t size);


#endif // TESTING_MEMORY_H
