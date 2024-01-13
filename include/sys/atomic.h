/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_ATOMIC_H
#define SYS_ATOMIC_H


/* prototypes */
int cas(int volatile *v, int old, int new);
void atomic_add(int volatile *v, int inc);


#endif // SYS_ATOMIC_H
