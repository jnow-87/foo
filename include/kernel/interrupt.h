/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_INTERRUPT_H
#define KERNEL_INTERRUPT_H


#include <sys/types.h>


/* macros */
#define INT_ALL ((int_type_t)(INT_GLOBAL))


/* types */
typedef uint8_t int_num_t;
typedef void (*int_hdlr_t)(int_num_t num, void *payload);

typedef enum int_type_t{
	INT_NONE = 0x0,
	INT_GLOBAL = 0x1,
} int_type_t;


/* prototypes */
int int_register(int_num_t num, int_hdlr_t hdlr, void *payload);
void int_release(int_num_t num);

void int_foretell(int_num_t num);

void int_khdlr(int_num_t num);


#endif // KERNEL_INTERRUPT_H
