/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARM_ATOMIC_H
#define ARM_ATOMIC_H


/* prototypes */
int arm_cas(int volatile *v, int old, int new);


#endif // ARM_ATOMIC_H
