/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_ATMEGA1284_H
#define ARCH_ATMEGA1284_H


#include <sys/const.h>


/**
 *	commonly required registers
 */
/* MCUCR */
// register
#define MCUCR			0x55

// bits
#define MCUCR_JTD		7
#define MCUCR_BODS		6
#define MCUCR_BODSE		5
#define MCUCR_PUD		4
#define MCUCR_IVSEL		1
#define MCUCR_IVCE		0

/* MCUSR */
// register
#define MCUSR			0x54

// bits
#define MCUSR_JTRF		4
#define MCUSR_WDRF		3
#define MCUSR_BORF		2
#define MCUSR_EXTRF		1
#define MCUSR_PROF		0

/* SREG */
// register
#define SREG			0x5f

// bits
#define SREG_I			7
#define SREG_T			6
#define SREG_H			5
#define SREG_S			4
#define SREG_V			3
#define SREG_N			2
#define SREG_Z			1
#define SREG_C			0

/* SP */
#define SPL				0x5d
#define SPH				0x5e

/* RAMPZ */
#define RAMPZ			0x5b

/* CLKPR */
// register
#define CLKPR			0x61

// bits
#define CLKPR_CLKPCE	7
#define CLKPR_CLKPS		0

/* SMCR */
// register
#define SMCR			0x53

// bits
#define SMCR_SM			1
#define SMCR_SE			0

/* PRR */
// register
#define PRR0			0x64
#define PRR1			0x65

// bits
#define PRR0_PRUSART0	1


/* GPIOR */
#define GPIOR0			0x3e
#define GPIOR1			0x4a
#define GPIOR2			0x4b

/* WDTCSR */
// register
#define WDTCSR			0x60

// bits
#define WDTCSR_WDIF		7
#define WDTCSR_WDIE		6
#define WDTCSR_WDP3		5
#define WDTCSR_WDCE		4
#define WDTCSR_WDE		3
#define WDTCSR_WDP2		2
#define WDTCSR_WDP1		1
#define WDTCSR_WDP0		0

/* UART */
// register
#define UCSR0A			0xc0
#define UDR0			0xc6


/**
 *	interrupt vectors
 */
// number of interrupts
#define INT_VECTORS			35

// interrupt numbers
#define INT_RESET			1
#define INT_0				2
#define INT_1				3
#define INT_2				4
#define INT_PCINT0			5
#define INT_PCINT1			6
#define INT_PCINT2			7
#define INT_PCINT3			8
#define INT_WATCHDOG		9
#define INT_TIMER2_COMPA	10
#define INT_TIMER2_COMPB	11
#define INT_TIMER2_OVFL		12
#define INT_TIMER1_CAPTURE	13
#define INT_TIMER1_COMPA	14
#define INT_TIMER1_COMPB	15
#define INT_TIMER1_OVFL		16
#define INT_TIMER0_COMPA	17
#define INT_TIMER0_COMPB	18
#define INT_TIMER0_OVFL		19
#define INT_SPI_TX			20
#define INT_USART0_RX		21
#define INT_USART0_UDRE		22
#define INT_USART0_TX		23
#define INT_ACOMP			24
#define INT_ADC				25
#define INT_EEPROM_RDY		26
#define INT_TWI				27
#define INT_SPM_RDY			28
#define INT_USART1_RX		29
#define INT_USART1_UDRE		30
#define INT_USART1_TX		31
#define INT_TIMER3_CAPTURE	32
#define INT_TIMER3_COMPA	33
#define INT_TIMER3_COMPB	34
#define INT_TIMER3_OVFL		35


/**
 * Device Configuration
 */
#define FLASH_SIZE			_128k
#define SRAM_SIZE			_16k
#define EEPROM_SIZE			_4k

#define	WATCHDOG_HZ			_128k


#endif // ARCH_ATMEGA1284_H
