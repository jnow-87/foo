#ifndef AVR_INTERRUPT_H
#define AVR_INTERRUPT_H


#include <arch/interrupt.h>
#include <arch/thread.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/compiler.h>


/* macros */
#define INT_ALL ((int_type_t)(INT_GLOBAL))
#define STR(s) #s
#define avr_int(int_num, call) \
	asm(".global __vector_" STR(int_num)); \
	asm(".set __vector_" STR(int_num) ", " STR(call)); \
	void call (void) __attribute__((signal));


/* types */
typedef enum int_type_t{
	INT_NONE = 0x0,
	INT_GLOBAL = 0x1,
} int_type_t;


/* prototypes */
int avr_int_enable(int_type_t mask);
int_type_t avr_int_enabled(void);


#endif // AVR_INTERRUPT_H
