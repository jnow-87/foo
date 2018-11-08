/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef AVR_ATOMIC_H
#define AVR_ATOMIC_H


/* prototypes */
int avr_cas(int volatile *v, int old, int new);


#endif // AVR_ATOMIC_H
