/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef AVR_INTERRUPT_H
#define AVR_INTERRUPT_H


#include <arch/interrupt.h>
#include <arch/thread.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/compiler.h>


/* macros */
#define INT_ALL ((int_type_t)(INT_GLOBAL))


/* types */
typedef enum int_type_t{
	INT_NONE = 0x0,
	INT_GLOBAL = 0x1,
} int_type_t;


/* prototypes */
int avr_int_register(int_num_t num, int_hdlr_t hdlr, void *data);
void avr_int_release(int_num_t num);

int_type_t avr_int_enable(int_type_t mask);
int_type_t avr_int_enabled(void);


#endif // AVR_INTERRUPT_H
