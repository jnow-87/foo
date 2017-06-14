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
typedef void (*isr_hdlr_t)(void);

typedef enum int_type_t{
	INT_NONE = 0x0,
	INT_GLOBAL = 0x1,
} int_type_t;

typedef enum int_num_t{
	// XXX: order has to match the hardware
	// 		interrupt vector table
	INT_RESET = 0,
	INT_0,
	INT_1,
	INT_2,
	INT_PCINT0,
	INT_PCINT1,
	INT_PCINT2,
	INT_PCINT3,
	INT_WATCHDOG,
	INT_TIMER2_COMPA,
	INT_TIMER2_COMPB,
	INT_TIMER2_OVFL,
	INT_TIMER1_CAMPTURE,
	INT_TIMER1_COMPA,
	INT_TIMER1_COMPB,
	INT_TIMER1_OVFL,
	INT_TIMER0_COMPA,
	INT_TIMER0_COMPB,
	INT_TIMER0_OVFL,
	INT_SPI_TX,
	INT_USART0_RX,
	INT_USART0_UDRE,
	INT_USART0_TX,
	INT_ACOMP,
	INT_ADC,
	INT_EEPROM_RDY,
	INT_TWI,
	INT_SPM_RDY,
	INT_USART1_RX,
	INT_USART1_UDRE,
	INT_USART1_TX,
	INT_TIMER3_CAMPTURE,
	INT_TIMER3_COMPA,
	INT_TIMER3_COMPB,
	INT_TIMER3_OVFL,

	// end of hardware interrupt vector table
	NINTERRUPTS,
} int_num_t;


/* prototypes */
struct thread_context_t *avr_int_hdlr(isr_hdlr_t addr, struct thread_context_t *tc);

errno_t avr_int_enable(int_type_t mask);
int_type_t avr_int_enabled(void);

errno_t avr_int_hdlr_register(int_num_t num, int_hdlr_t hdlr);
errno_t avr_int_hdlr_release(int_num_t num);


#endif // AVR_INTERRUPT_H
