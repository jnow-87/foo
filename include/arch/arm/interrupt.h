/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARM_INTERRUPT_H
#define ARM_INTERRUPT_H


/* macros */
#define INT_ALL ((int_type_t)(INT_GLOBAL))


/* types */
typedef enum int_type_t{
	INT_NONE = 0x0,
	INT_GLOBAL = 0x1,
} int_type_t;


#endif // ARM_INTERRUPT_H
