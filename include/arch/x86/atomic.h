/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86_ATOMIC_H
#define X86_ATOMIC_H


/* prototypes */
int x86_cas(int volatile *v, int old, int new);
void x86_atomic_add(int volatile *v, int inc);


#endif // X86_ATOMIC_H
