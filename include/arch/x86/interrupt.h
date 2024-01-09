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
#include <sys/types.h>


/* prototypes */
bool x86_int_enable(bool en);
bool x86_int_enabled(void);

x86_hw_op_t *x86_int_op(void);


#endif // X86_INTERRUPT_H
