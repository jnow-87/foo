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

/**
 * \brief	Macro intented to register avr interrupt handlers that are
 * 			not supposed to go through the kernel interrupt handling
 * 			process, rather than handling the interrupt directly.
 *
 * 			The macro generates a function which preserves the global
 * 			errno variable, marks this function as interrupt routine
 * 			and registers it as handler for	the given interrupt number.
 *
 * \param	int_num		interrupt number to register for
 * \param	call		callback to register
 */
#define avr_int(int_num, call) \
	void __vector_##call() __attribute__((signal)); \
	void __vector_##call(){ \
		errno_t e;\
		\
		\
		e = errno; \
		call(); \
		errno = e; \
	} \
	\
	asm(".global __vector_" STRGIFY(int_num)); \
	asm(".set __vector_" STRGIFY(int_num) ", __vector_" STRGIFY(call));


/* types */
typedef enum int_type_t{
	INT_NONE = 0x0,
	INT_GLOBAL = 0x1,
} int_type_t;

typedef uint8_t int_num_t;


/* prototypes */
int_type_t avr_int_enable(int_type_t mask);
int_type_t avr_int_enabled(void);


#endif // AVR_INTERRUPT_H
