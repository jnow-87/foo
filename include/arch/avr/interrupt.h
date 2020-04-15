/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef AVR_INTERRUPT_H
#define AVR_INTERRUPT_H


#include <arch/interrupt.h>


/* prototypes */
int avr_int_register(int_num_t num, int_hdlr_t hdlr, void *data);
void avr_int_release(int_num_t num);

void avr_int_call(int_num_t num);

int_type_t avr_int_enable(int_type_t mask);
int_type_t avr_int_enabled(void);


#endif // AVR_INTERRUPT_H
