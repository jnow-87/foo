/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef AVR_INTERRUPT_H
#define AVR_INTERRUPT_H


#include <kernel/interrupt.h>
#include <sys/types.h>


/* prototypes */
bool avr_int_enable(bool en);
bool avr_int_enabled(void);


#endif // AVR_INTERRUPT_H
