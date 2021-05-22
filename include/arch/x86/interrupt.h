/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86_INTERRUPT_H
#define X86_INTERRUPT_H


#include <arch/x86/hardware.h>
#include <kernel/interrupt.h>


/* prototypes */
int_type_t x86_int_enabled(void);
int_type_t x86_int_enable(int_type_t mask);

x86_hw_op_t *x86_int_op(void);
void x86_int_trigger(int_num_t num);


#endif // X86_INTERRUPT_H
