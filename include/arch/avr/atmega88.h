/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_ATMEGA88_H
#define ARCH_ATMEGA88_H


#include <sys/const.h>


/**
 *	commonly required registers
 */
/* MCUCR */
// register
#define MCUCR			0x55

// bits
#define MCUCR_BODS		6
#define MCUCR_BODSE		5
#define MCUCR_PUD		4
#define MCUCR_IVSEL		1
#define MCUCR_IVCE		0

/* MCUSR */
// register
#define MCUSR			0x54

// bits
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
#define PRR0			0x64

// bits
#define PRR0_PRUSART0	1


/**
 *	interrupt vectors
 */
// number of interrupts
#define INT_VECTORS			26

// interrupt numbers
#define INT_0				1
#define INT_1				2
#define INT_PCINT0			3
#define INT_PCINT1			4
#define INT_PCINT2			5
#define INT_WATCHDOG		6
#define INT_TIMER2_COMPA	7
#define INT_TIMER2_COMPB	8
#define INT_TIMER2_OVFL		9
#define INT_TIMER1_CAPTURE	10
#define INT_TIMER1_COMPA	11
#define INT_TIMER1_COMPB	12
#define INT_TIMER1_OVFL		13
#define INT_TIMER0_COMPA	14
#define INT_TIMER0_COMPB	15
#define INT_TIMER0_OVFL		16
#define INT_SPI_TX			17
#define INT_USART0_RX		18
#define INT_USART0_UDRE		19
#define INT_USART0_TX		20
#define INT_ADC				21
#define INT_EEPROM_RDY		22
#define INT_ACOMP			23
#define INT_TWI				24
#define INT_SPM_RDY			25


/**
 * Device Configuration
 */
#define FLASH_SIZE			_8k
#define SRAM_SIZE			_1k
#define EEPROM_SIZE			512

#define	WATCHDOG_HZ			_128k


#endif // ARCH_ATMEGA88_H
