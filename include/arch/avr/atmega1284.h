/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_ATMEGA1284_H
#define ARCH_ATMEGA1284_H


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


/**
 *	interrupt vectors
 */
// interrupt numbers
#define INT_RESET			0
#define INT_0				1
#define INT_1				2
#define INT_2				3
#define INT_PCINT0			4
#define INT_PCINT1			5
#define INT_PCINT2			6
#define INT_PCINT3			7
#define INT_WATCHDOG		8
#define INT_TIMER2_COMPA	9
#define INT_TIMER2_COMPB	10
#define INT_TIMER2_OVFL		11
#define INT_TIMER1_CAPTURE	12
#define INT_TIMER1_COMPA	13
#define INT_TIMER1_COMPB	14
#define INT_TIMER1_OVFL		15
#define INT_TIMER0_COMPA	16
#define INT_TIMER0_COMPB	17
#define INT_TIMER0_OVFL		18
#define INT_SPI_TX			19
#define INT_USART0_RX		20
#define INT_USART0_UDRE		21
#define INT_USART0_TX		22
#define INT_ACOMP			23
#define INT_ADC				24
#define INT_EEPROM_RDY		25
#define INT_TWI				26
#define INT_SPM_RDY			27
#define INT_USART1_RX		28
#define INT_USART1_UDRE		29
#define INT_USART1_TX		30
#define INT_TIMER3_CAPTURE	31
#define INT_TIMER3_COMPA	32
#define INT_TIMER3_COMPB	33
#define INT_TIMER3_OVFL		34

// software interrupts
#define INT_SYSCALL			35
#define INT_IOVFL			36

#define INT_NUM_HWINTS		37


#endif // ARCH_ATMEGA1284_H
