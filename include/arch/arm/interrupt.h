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
#define arm_int(int_num, call) \
	void __vector_##call(); \
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


#endif // ARM_INTERRUPT_H
