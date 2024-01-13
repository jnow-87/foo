/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef AVR_ATOMIC_H
#define AVR_ATOMIC_H


#include <arch/arch.h>


/* prototypes */
int avr_atomic(atomic_t op, void *param);


#endif // AVR_ATOMIC_H
