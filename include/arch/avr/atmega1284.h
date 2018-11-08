/*
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARCH_ATMEGA1284_H
#define ARCH_ATMEGA1284_H


#include <sys/const.h>


/**
 *	General Purpose Working Registes
 */
/* R1 */
// register
#define R1			0x1

/* R2 */
// register
#define R2			0x2

/* R3 */
// register
#define R3			0x3

/* R4 */
// register
#define R4			0x4

/* R5 */
// register
#define R5			0x5

/* R6 */
// register
#define R6			0x6

/* R7 */
// register
#define R7			0x7

/* R8 */
// register
#define R8			0x8

/* R9 */
// register
#define R9			0x9

/* R10 */
// register
#define R10			0xa

/* R11 */
// register
#define R11			0xb

/* R12 */
// register
#define R12			0xc

/* R13 */
// register
#define R13			0xd

/* R14 */
// register
#define R14			0xe

/* R15 */
// register
#define R15			0xf

/* R16 */
// register
#define R16			0x10

/* R17 */
// register
#define R17			0x11

/* R18 */
// register
#define R18			0x12

/* R19 */
// register
#define R19			0x13

/* R20 */
// register
#define R20			0x14

/* R21 */
// register
#define R21			0x15

/* R22 */
// register
#define R22			0x16

/* R23 */
// register
#define R23			0x17

/* R24 */
// register
#define R24			0x18

/* R25 */
// register
#define R25			0x19

/* R26 */
// register
#define R26			0x1a

/* R27 */
// register
#define R27			0x1b

/* R28 */
// register
#define R28			0x1c

/* R29 */
// register
#define R29			0x1d

/* R30 */
// register
#define R30			0x1e

/* R31 */
// register
#define R31			0x1f

/**
 *	MCU General Control and Status
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

// masks
#define MCUCR_JTD_MASK	0x1
#define MCUCR_BODS_MASK	0x1
#define MCUCR_BODSE_MASK	0x1
#define MCUCR_PUD_MASK	0x1
#define MCUCR_IVSEL_MASK	0x1
#define MCUCR_IVCE_MASK	0x1

/* MCUSR */
// register
#define MCUSR			0x54

// bits
#define MCUSR_JTRF		4
#define MCUSR_WDRF		3
#define MCUSR_BORF		2
#define MCUSR_EXTRF		1
#define MCUSR_PROF		0

// masks
#define MCUSR_JTRF_MASK	0x1
#define MCUSR_WDRF_MASK	0x1
#define MCUSR_BORF_MASK	0x1
#define MCUSR_EXTRF_MASK	0x1
#define MCUSR_PROF_MASK	0x1

/* SREG */
// register
#define SREG			0x5f

// bits
#define SREG_I		7
#define SREG_T		6
#define SREG_H		5
#define SREG_S		4
#define SREG_V		3
#define SREG_N		2
#define SREG_Z		1
#define SREG_C		0

// masks
#define SREG_I_MASK	0x1
#define SREG_T_MASK	0x1
#define SREG_H_MASK	0x1
#define SREG_S_MASK	0x1
#define SREG_V_MASK	0x1
#define SREG_N_MASK	0x1
#define SREG_Z_MASK	0x1
#define SREG_C_MASK	0x1

/* SPL */
// register
#define SPL			0x5d

/* SPH */
// register
#define SPH			0x5e

/* SMCR */
// register
#define SMCR			0x53

// bits
#define SMCR_SM		1
#define SMCR_SE		0

// masks
#define SMCR_SM_MASK	0x7
#define SMCR_SE_MASK	0x1

/* SPMCSR */
// register
#define SPMCSR			0x57

// bits
#define SPMCSR_SPMIE		7
#define SPMCSR_RWWSB		6
#define SPMCSR_SIGRD		5
#define SPMCSR_RWWSRE		4
#define SPMCSR_BLBSET		3
#define SPMCSR_PGWRT		2
#define SPMCSR_PGERS		1
#define SPMCSR_SPMEN		0

// masks
#define SPMCSR_SPMIE_MASK	0x1
#define SPMCSR_RWWSB_MASK	0x1
#define SPMCSR_SIGRD_MASK	0x1
#define SPMCSR_RWWSRE_MASK	0x1
#define SPMCSR_BLBSET_MASK	0x1
#define SPMCSR_PGWRT_MASK	0x1
#define SPMCSR_PGERS_MASK	0x0
#define SPMCSR_SPMEN_MASK	0x0

/* OCDR */
// register
#define OCDR			0x51

// bits
#define OCDR_VALUE		0

// masks
#define OCDR_VALUE_MASK	0xff

/* RAMPZ */
// register
#define RAMPZ			0x5b

/* DDRA */
// register
#define DDRA			0x21

/* DDRB */
// register
#define DDRB			0x24

/* DDRC */
// register
#define DDRC			0x27

/* DDRD */
// register
#define DDRD			0x2a

/* PINA */
// register
#define PINA			0x20

/* PINB */
// register
#define PINB			0x23

/* PINC */
// register
#define PINC			0x26

/* PIND */
// register
#define PIND			0x29

/* PORTA */
// register
#define PORTA			0x22

/* PORTB */
// register
#define PORTB			0x25

/* PORTC */
// register
#define PORTC			0x28

/* PORTD */
// register
#define PORTD			0x2b

/* GPIOR0 */
// register
#define GPIOR0			0x3e

/* GPIOR1 */
// register
#define GPIOR1			0x4a

/* GPIOR2 */
// register
#define GPIOR2			0x4b

/* PCICR */
// register
#define PCICR			0x68

// bits
#define PCICR_PCIE3		3
#define PCICR_PCIE2		2
#define PCICR_PCIE1		1
#define PCICR_PCIE0		0

// masks
#define PCICR_PCIE3_MASK	0x1
#define PCICR_PCIE2_MASK	0x1
#define PCICR_PCIE1_MASK	0x1
#define PCICR_PCIE0_MASK	0x1

/* PCMSK0 */
// register
#define PCMSK0			0x6b

// bits
#define PCMSK0_PCINT7		7
#define PCMSK0_PCINT6		6
#define PCMSK0_PCINT5		5
#define PCMSK0_PCINT4		4
#define PCMSK0_PCINT3		3
#define PCMSK0_PCINT2		2
#define PCMSK0_PCINT1		1
#define PCMSK0_PCINT0		0

// masks
#define PCMSK0_PCINT7_MASK	0x1
#define PCMSK0_PCINT6_MASK	0x1
#define PCMSK0_PCINT5_MASK	0x1
#define PCMSK0_PCINT4_MASK	0x1
#define PCMSK0_PCINT3_MASK	0x1
#define PCMSK0_PCINT2_MASK	0x1
#define PCMSK0_PCINT1_MASK	0x1
#define PCMSK0_PCINT0_MASK	0x1

/* PCMSK1 */
// register
#define PCMSK1			0x6c

// bits
#define PCMSK1_PCINT15		7
#define PCMSK1_PCINT14		6
#define PCMSK1_PCINT13		5
#define PCMSK1_PCINT12		4
#define PCMSK1_PCINT11		3
#define PCMSK1_PCINT10		2
#define PCMSK1_PCINT9		1
#define PCMSK1_PCINT8		0

// masks
#define PCMSK1_PCINT15_MASK	0x1
#define PCMSK1_PCINT14_MASK	0x1
#define PCMSK1_PCINT13_MASK	0x1
#define PCMSK1_PCINT12_MASK	0x1
#define PCMSK1_PCINT11_MASK	0x1
#define PCMSK1_PCINT10_MASK	0x1
#define PCMSK1_PCINT9_MASK	0x1
#define PCMSK1_PCINT8_MASK	0x1

/* PCMSK2 */
// register
#define PCMSK2			0x6d

// bits
#define PCMSK2_PCINT23		7
#define PCMSK2_PCINT22		6
#define PCMSK2_PCINT21		5
#define PCMSK2_PCINT20		4
#define PCMSK2_PCINT19		3
#define PCMSK2_PCINT18		2
#define PCMSK2_PCINT17		1
#define PCMSK2_PCINT16		0

// masks
#define PCMSK2_PCINT23_MASK	0x1
#define PCMSK2_PCINT22_MASK	0x1
#define PCMSK2_PCINT21_MASK	0x1
#define PCMSK2_PCINT20_MASK	0x1
#define PCMSK2_PCINT19_MASK	0x1
#define PCMSK2_PCINT18_MASK	0x1
#define PCMSK2_PCINT17_MASK	0x1
#define PCMSK2_PCINT16_MASK	0x1

/* PCMSK3 */
// register
#define PCMSK3			0x73

// bits
#define PCMSK3_PCINT31		7
#define PCMSK3_PCINT30		6
#define PCMSK3_PCINT29		5
#define PCMSK3_PCINT28		4
#define PCMSK3_PCINT27		3
#define PCMSK3_PCINT26		2
#define PCMSK3_PCINT25		1
#define PCMSK3_PCINT24		0

// masks
#define PCMSK3_PCINT31_MASK	0x1
#define PCMSK3_PCINT30_MASK	0x1
#define PCMSK3_PCINT29_MASK	0x1
#define PCMSK3_PCINT28_MASK	0x1
#define PCMSK3_PCINT27_MASK	0x1
#define PCMSK3_PCINT26_MASK	0x1
#define PCMSK3_PCINT25_MASK	0x1
#define PCMSK3_PCINT24_MASK	0x1

/* PCIFR */
// register
#define PCIFR			0x3b

// bits
#define PCIFR_PCIF3		3
#define PCIFR_PCIF2		2
#define PCIFR_PCIF1		1
#define PCIFR_PCIF0		0

// masks
#define PCIFR_PCIF3_MASK	0x1
#define PCIFR_PCIF2_MASK	0x1
#define PCIFR_PCIF1_MASK	0x1
#define PCIFR_PCIF0_MASK	0x1

/* EICRA */
// register
#define EICRA			0x69

// bits
#define EICRA_ISC21		5
#define EICRA_ISC20		4
#define EICRA_ISC11		3
#define EICRA_ISC10		2
#define EICRA_ISC01		1
#define EICRA_ISC00		0

// masks
#define EICRA_ISC21_MASK	0x1
#define EICRA_ISC20_MASK	0x1
#define EICRA_ISC11_MASK	0x1
#define EICRA_ISC10_MASK	0x1
#define EICRA_ISC01_MASK	0x1
#define EICRA_ISC00_MASK	0x1

/* EIMSK */
// register
#define EIMSK			0x3d

// bits
#define EIMSK_INT2		2
#define EIMSK_INT1		1
#define EIMSK_INT0		0

// masks
#define EIMSK_INT2_MASK	0x1
#define EIMSK_INT1_MASK	0x1
#define EIMSK_INT0_MASK	0x1

/* EIFR */
// register
#define EIFR			0x3c

// bits
#define EIFR_INTF2		2
#define EIFR_INTF1		1
#define EIFR_INTF0		0

// masks
#define EIFR_INTF2_MASK	0x1
#define EIFR_INTF1_MASK	0x1
#define EIFR_INTF0_MASK	0x1

/* CLKPR */
// register
#define CLKPR			0x61

// bits
#define CLKPR_CLKPCE		7
#define CLKPR_CLKPS		0

// masks
#define CLKPR_CLKPCE_MASK	0x1
#define CLKPR_CLKPS_MASK	0xf

/* OSCCAL */
// register
#define OSCCAL			0x66

// bits
#define OSCCAL_VALUE		0

// masks
#define OSCCAL_VALUE_MASK	0xff

/* PRR0 */
// register
#define PRR0			0x64

// bits
#define PRR0_PRTWI		7
#define PRR0_PRTIM2		6
#define PRR0_PRTIM0		5
#define PRR0_PRUSART1		4
#define PRR0_PRTIM1		3
#define PRR0_PRSPI		2
#define PRR0_PRUSART0		1
#define PRR0_PRADC		0

// masks
#define PRR0_PRTWI_MASK	0x1
#define PRR0_PRTIM2_MASK	0x1
#define PRR0_PRTIM0_MASK	0x1
#define PRR0_PRUSART1_MASK	0x1
#define PRR0_PRTIM1_MASK	0x1
#define PRR0_PRSPI_MASK	0x1
#define PRR0_PRUSART0_MASK	0x1
#define PRR0_PRADC_MASK	0x1

/* PRR1 */
// register
#define PRR1			0x65

// bits
#define PRR1_PRTIM3		0

// masks
#define PRR1_PRTIM3_MASK	0x1

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

// masks
#define WDTCSR_WDIF_MASK	0x1
#define WDTCSR_WDIE_MASK	0x1
#define WDTCSR_WDP3_MASK	0x1
#define WDTCSR_WDCE_MASK	0x1
#define WDTCSR_WDE_MASK	0x1
#define WDTCSR_WDP2_MASK	0x1
#define WDTCSR_WDP1_MASK	0x1
#define WDTCSR_WDP0_MASK	0x1

/**
 *	Universal Synchronous and Asynchronous Serial Receiver and Transmitter (USART)
 */
/* UCSR0A */
// register
#define UCSR0A			0xc0

// bits
#define UCSR0A_RXC		7
#define UCSR0A_TXC		6
#define UCSR0A_UDRE		5
#define UCSR0A_FE		4
#define UCSR0A_DOR		3
#define UCSR0A_UPE		2
#define UCSR0A_U2X		1
#define UCSR0A_MPCM		0

// masks
#define UCSR0A_RXC_MASK	0x1
#define UCSR0A_TXC_MASK	0x1
#define UCSR0A_UDRE_MASK	0x1
#define UCSR0A_FE_MASK	0x1
#define UCSR0A_DOR_MASK	0x1
#define UCSR0A_UPE_MASK	0x1
#define UCSR0A_U2X_MASK	0x1
#define UCSR0A_MPCM_MASK	0x1

/* UCSR0B */
// register
#define UCSR0B			0xc1

// bits
#define UCSR0B_RXCIE		7
#define UCSR0B_TXCIE		6
#define UCSR0B_UDRIE		5
#define UCSR0B_RXEN		4
#define UCSR0B_TXEN		3
#define UCSR0B_UCSZ2		2
#define UCSR0B_RXB8		1
#define UCSR0B_TXB8		0

// masks
#define UCSR0B_RXCIE_MASK	0x1
#define UCSR0B_TXCIE_MASK	0x1
#define UCSR0B_UDRIE_MASK	0x1
#define UCSR0B_RXEN_MASK	0x1
#define UCSR0B_TXEN_MASK	0x1
#define UCSR0B_UCSZ2_MASK	0x1
#define UCSR0B_RXB8_MASK	0x1
#define UCSR0B_TXB8_MASK	0x1

/* UCSR0C */
// register
#define UCSR0C			0xc2

// bits
#define UCSR0C_UMSEL1		7
#define UCSR0C_UMSEL0		6
#define UCSR0C_UPM1		5
#define UCSR0C_UPM0		4
#define UCSR0C_USBS		3
#define UCSR0C_UCSZ1		2
#define UCSR0C_UCSZ0		1
#define UCSR0C_UCPOL		0

// masks
#define UCSR0C_UMSEL1_MASK	0x1
#define UCSR0C_UMSEL0_MASK	0x1
#define UCSR0C_UPM1_MASK	0x1
#define UCSR0C_UPM0_MASK	0x1
#define UCSR0C_USBS_MASK	0x1
#define UCSR0C_UCSZ1_MASK	0x1
#define UCSR0C_UCSZ0_MASK	0x1
#define UCSR0C_UCPOL_MASK	0x1

/* UBRR0L */
// register
#define UBRR0L			0xc4

// bits
#define UBRR0L_Low		0

// masks
#define UBRR0L_Low_MASK	0xff

/* UBRR0H */
// register
#define UBRR0H			0xc5

// bits
#define UBRR0H_High		0

// masks
#define UBRR0H_High_MASK	0xf

/* UDR0 */
// register
#define UDR0			0xc6

// bits
#define UDR0_Data		0

// masks
#define UDR0_Data_MASK	0xff

/* UCSR1A */
// register
#define UCSR1A			0xc8

// bits
#define UCSR1A_RXC		7
#define UCSR1A_TXC		6
#define UCSR1A_UDRE		5
#define UCSR1A_FE		4
#define UCSR1A_DOR		3
#define UCSR1A_UPE		2
#define UCSR1A_U2X		1
#define UCSR1A_MPCM		0

// masks
#define UCSR1A_RXC_MASK	0x1
#define UCSR1A_TXC_MASK	0x1
#define UCSR1A_UDRE_MASK	0x1
#define UCSR1A_FE_MASK	0x1
#define UCSR1A_DOR_MASK	0x1
#define UCSR1A_UPE_MASK	0x1
#define UCSR1A_U2X_MASK	0x1
#define UCSR1A_MPCM_MASK	0x1

/* UCSR1B */
// register
#define UCSR1B			0xc9

// bits
#define UCSR1B_RXCIE		7
#define UCSR1B_TXCIE		6
#define UCSR1B_UDRIE		5
#define UCSR1B_RXEN		4
#define UCSR1B_TXEN		3
#define UCSR1B_UCSZ2		2
#define UCSR1B_RXB8		1
#define UCSR1B_TXB8		0

// masks
#define UCSR1B_RXCIE_MASK	0x1
#define UCSR1B_TXCIE_MASK	0x1
#define UCSR1B_UDRIE_MASK	0x1
#define UCSR1B_RXEN_MASK	0x1
#define UCSR1B_TXEN_MASK	0x1
#define UCSR1B_UCSZ2_MASK	0x1
#define UCSR1B_RXB8_MASK	0x1
#define UCSR1B_TXB8_MASK	0x1

/* UCSR1C */
// register
#define UCSR1C			0xca

// bits
#define UCSR1C_UMSEL1		7
#define UCSR1C_UMSEL0		6
#define UCSR1C_UPM1		5
#define UCSR1C_UPM0		4
#define UCSR1C_USBS		3
#define UCSR1C_UCSZ1		2
#define UCSR1C_UCSZ0		1
#define UCSR1C_UCPOL		0

// masks
#define UCSR1C_UMSEL1_MASK	0x1
#define UCSR1C_UMSEL0_MASK	0x1
#define UCSR1C_UPM1_MASK	0x1
#define UCSR1C_UPM0_MASK	0x1
#define UCSR1C_USBS_MASK	0x1
#define UCSR1C_UCSZ1_MASK	0x1
#define UCSR1C_UCSZ0_MASK	0x1
#define UCSR1C_UCPOL_MASK	0x1

/* UBRR1L */
// register
#define UBRR1L			0xcc

// bits
#define UBRR1L_Low		0

// masks
#define UBRR1L_Low_MASK	0xff

/* UBRR1H */
// register
#define UBRR1H			0xcd

// bits
#define UBRR1H_High		0

// masks
#define UBRR1H_High_MASK	0xf

/* UDR1 */
// register
#define UDR1			0xce

// bits
#define UDR1_Data		0

// masks
#define UDR1_Data_MASK	0xff

/**
 *	Two Wire Interface (TWI)
 */
/* TWCR */
// register
#define TWCR			0xbc

// bits
#define TWCR_TWINT		7
#define TWCR_TWEA		6
#define TWCR_TWSTA		5
#define TWCR_TWSTO		4
#define TWCR_TWWC		3
#define TWCR_TWEN		2
#define TWCR_TWIE		0

// masks
#define TWCR_TWINT_MASK	0x1
#define TWCR_TWEA_MASK	0x1
#define TWCR_TWSTA_MASK	0x1
#define TWCR_TWSTO_MASK	0x1
#define TWCR_TWWC_MASK	0x1
#define TWCR_TWEN_MASK	0x1
#define TWCR_TWIE_MASK	0x1

/* TWSR */
// register
#define TWSR			0xb9

// bits
#define TWSR_TWS7		7
#define TWSR_TWS6		6
#define TWSR_TWS5		5
#define TWSR_TWS4		4
#define TWSR_TWS3		3
#define TWSR_TWPS1		1
#define TWSR_TWPS0		0

// masks
#define TWSR_TWS7_MASK	0x1
#define TWSR_TWS6_MASK	0x1
#define TWSR_TWS5_MASK	0x1
#define TWSR_TWS4_MASK	0x1
#define TWSR_TWS3_MASK	0x1
#define TWSR_TWPS1_MASK	0x1
#define TWSR_TWPS0_MASK	0x1

/* TWDR */
// register
#define TWDR			0xbb

// bits
#define TWDR_Data		0

// masks
#define TWDR_Data_MASK	0xff

/* TWBR */
// register
#define TWBR			0xb8

// bits
#define TWBR_BR		0

// masks
#define TWBR_BR_MASK	0xff

/* TWAR */
// register
#define TWAR			0xba

// bits
#define TWAR_TWA		1
#define TWAR_TWGCE		0

// masks
#define TWAR_TWA_MASK	0x7f
#define TWAR_TWGCE_MASK	0x1

/* TWAMR */
// register
#define TWAMR			0xbd

// bits
#define TWAMR_TWAM		1

// masks
#define TWAMR_TWAM_MASK	0x7f

/**
 *	Serial Peripheral Interface (SPI)
 */
/* SPCR */
// register
#define SPCR			0x4c

// bits
#define SPCR_SPIE0		7
#define SPCR_SPE0		6
#define SPCR_DORD0		5
#define SPCR_MSTR0		4
#define SPCR_CPOL0		3
#define SPCR_CPHA0		2
#define SPCR_SPR0		0

// masks
#define SPCR_SPIE0_MASK	0x1
#define SPCR_SPE0_MASK	0x1
#define SPCR_DORD0_MASK	0x1
#define SPCR_MSTR0_MASK	0x1
#define SPCR_CPOL0_MASK	0x1
#define SPCR_CPHA0_MASK	0x1
#define SPCR_SPR0_MASK	0x3

/* SPSR */
// register
#define SPSR			0x4d

// bits
#define SPSR_SPIF0		7
#define SPSR_WCOL0		6
#define SPSR_SPI2X0		0

// masks
#define SPSR_SPIF0_MASK	0x1
#define SPSR_WCOL0_MASK	0x1
#define SPSR_SPI2X0_MASK	0x1

/* SPDR */
// register
#define SPDR			0x4e

// bits
#define SPDR_DATA		0

// masks
#define SPDR_DATA_MASK	0xff

/**
 *	Timer/Counter 0 (8-bit)
 */
/* TCCR0A */
// register
#define TCCR0A			0x44

// bits
#define TCCR0A_COMA		6
#define TCCR0A_COMB		4
#define TCCR0A_WGM1		1
#define TCCR0A_WGM0		0

// masks
#define TCCR0A_COMA_MASK	0x3
#define TCCR0A_COMB_MASK	0x3
#define TCCR0A_WGM1_MASK	0x1
#define TCCR0A_WGM0_MASK	0x1

/* TCCR0B */
// register
#define TCCR0B			0x45

// bits
#define TCCR0B_FOCA		7
#define TCCR0B_FOCB		6
#define TCCR0B_WGM2		3
#define TCCR0B_CS		0

// masks
#define TCCR0B_FOCA_MASK	0x1
#define TCCR0B_FOCB_MASK	0x1
#define TCCR0B_WGM2_MASK	0x1
#define TCCR0B_CS_MASK	0x7

/* TCNT0 */
// register
#define TCNT0			0x46

// bits
#define TCNT0_CNT		0

// masks
#define TCNT0_CNT_MASK	0xff

/* OCR0A */
// register
#define OCR0A			0x47

// bits
#define OCR0A_VALUE		0

// masks
#define OCR0A_VALUE_MASK	0xff

/* OCR0B */
// register
#define OCR0B			0x48

// bits
#define OCR0B_VALUE		0

// masks
#define OCR0B_VALUE_MASK	0xff

/* TIMSK0 */
// register
#define TIMSK0			0x6e

// bits
#define TIMSK0_OCIE0B		2
#define TIMSK0_OCIE0A		1
#define TIMSK0_TOIE0		0

// masks
#define TIMSK0_OCIE0B_MASK	0x1
#define TIMSK0_OCIE0A_MASK	0x1
#define TIMSK0_TOIE0_MASK	0x1

/* TIFR0 */
// register
#define TIFR0			0x35

// bits
#define TIFR0_OCF0B		2
#define TIFR0_OCF0A		1
#define TIFR0_TOV0		0

// masks
#define TIFR0_OCF0B_MASK	0x1
#define TIFR0_OCF0A_MASK	0x1
#define TIFR0_TOV0_MASK	0x1

/**
 *	Timer/Counter 1 (16-bit)
 */
/* TCCR1A */
// register
#define TCCR1A			0x80

// bits
#define TCCR1A_COMA		6
#define TCCR1A_COMB		4
#define TCCR1A_WGM1		1
#define TCCR1A_WGM0		0

// masks
#define TCCR1A_COMA_MASK	0x3
#define TCCR1A_COMB_MASK	0x3
#define TCCR1A_WGM1_MASK	0x1
#define TCCR1A_WGM0_MASK	0x1

/* TCCR1B */
// register
#define TCCR1B			0x81

// bits
#define TCCR1B_ICNC		7
#define TCCR1B_ICES		6
#define TCCR1B_WGM3		4
#define TCCR1B_WGM2		3
#define TCCR1B_CS		0

// masks
#define TCCR1B_ICNC_MASK	0x1
#define TCCR1B_ICES_MASK	0x1
#define TCCR1B_WGM3_MASK	0x1
#define TCCR1B_WGM2_MASK	0x1
#define TCCR1B_CS_MASK	0x7

/* TCCR1C */
// register
#define TCCR1C			0x82

// bits
#define TCCR1C_FOCA		7
#define TCCR1C_FOCB		6

// masks
#define TCCR1C_FOCA_MASK	0x1
#define TCCR1C_FOCB_MASK	0x1

/* TCNT1L */
// register
#define TCNT1L			0x84

// bits
#define TCNT1L_CNTL		0

// masks
#define TCNT1L_CNTL_MASK	0xff

/* TCNT1H */
// register
#define TCNT1H			0x85

// bits
#define TCNT1H_CNTH		0

// masks
#define TCNT1H_CNTH_MASK	0xff

/* ICR1 */
// register
#define ICR1			0x86

/* OCR1A */
// register
#define OCR1A			0x88

/* OCR1B */
// register
#define OCR1B			0x8a

/* TIMSK1 */
// register
#define TIMSK1			0x6f

// bits
#define TIMSK1_ICIE1		5
#define TIMSK1_OCIE1B		2
#define TIMSK1_OCIE1A		1
#define TIMSK1_TOIE1		0

// masks
#define TIMSK1_ICIE1_MASK	0x1
#define TIMSK1_OCIE1B_MASK	0x1
#define TIMSK1_OCIE1A_MASK	0x1
#define TIMSK1_TOIE1_MASK	0x1

/* TIFR1 */
// register
#define TIFR1			0x36

// bits
#define TIFR1_ICF1		5
#define TIFR1_OCF1B		2
#define TIFR1_OCF1A		1
#define TIFR1_TOV1		0

// masks
#define TIFR1_ICF1_MASK	0x1
#define TIFR1_OCF1B_MASK	0x1
#define TIFR1_OCF1A_MASK	0x1
#define TIFR1_TOV1_MASK	0x1

/**
 *	Timer/Counter 2 (8-bit)
 */
/* TCCR2A */
// register
#define TCCR2A			0xb0

// bits
#define TCCR2A_COMA		6
#define TCCR2A_COMB		4
#define TCCR2A_WGM1		1
#define TCCR2A_WGM0		0

// masks
#define TCCR2A_COMA_MASK	0x3
#define TCCR2A_COMB_MASK	0x3
#define TCCR2A_WGM1_MASK	0x1
#define TCCR2A_WGM0_MASK	0x1

/* TCCR2B */
// register
#define TCCR2B			0xb1

// bits
#define TCCR2B_FOCA		7
#define TCCR2B_FOCB		6
#define TCCR2B_WGM2		3
#define TCCR2B_CS		0

// masks
#define TCCR2B_FOCA_MASK	0x1
#define TCCR2B_FOCB_MASK	0x1
#define TCCR2B_WGM2_MASK	0x1
#define TCCR2B_CS_MASK	0x7

/* TCNT2 */
// register
#define TCNT2			0xb2

// bits
#define TCNT2_CNT		0

// masks
#define TCNT2_CNT_MASK	0xff

/* OCR2A */
// register
#define OCR2A			0xb3

// bits
#define OCR2A_VALUE		0

// masks
#define OCR2A_VALUE_MASK	0xff

/* OCR2B */
// register
#define OCR2B			0xb4

// bits
#define OCR2B_VALUE		0

// masks
#define OCR2B_VALUE_MASK	0xff

/* ASSR */
// register
#define ASSR			0xb6

// bits
#define ASSR_EXCLK		6
#define ASSR_AS2		5
#define ASSR_TCNUB		4
#define ASSR_OCRAUB		3
#define ASSR_OCRBUB		2
#define ASSR_TCRAUB		1
#define ASSR_TCRBUB		0

// masks
#define ASSR_EXCLK_MASK	0x1
#define ASSR_AS2_MASK	0x1
#define ASSR_TCNUB_MASK	0x1
#define ASSR_OCRAUB_MASK	0x1
#define ASSR_OCRBUB_MASK	0x1
#define ASSR_TCRAUB_MASK	0x1
#define ASSR_TCRBUB_MASK	0x1

/* GTCCR */
// register
#define GTCCR			0x43

// bits
#define GTCCR_TSM		7
#define GTCCR_PSRASY		1
#define GTCCR_PSRSYNC		0

// masks
#define GTCCR_TSM_MASK	0x1
#define GTCCR_PSRASY_MASK	0x1
#define GTCCR_PSRSYNC_MASK	0x1

/* TIMSK2 */
// register
#define TIMSK2			0x70

// bits
#define TIMSK2_OCIE2B		2
#define TIMSK2_OCIE2A		1
#define TIMSK2_TOIE2		0

// masks
#define TIMSK2_OCIE2B_MASK	0x1
#define TIMSK2_OCIE2A_MASK	0x1
#define TIMSK2_TOIE2_MASK	0x1

/* TIFR2 */
// register
#define TIFR2			0x37

// bits
#define TIFR2_OCF2B		2
#define TIFR2_OCF2A		1
#define TIFR2_TOV2		0

// masks
#define TIFR2_OCF2B_MASK	0x1
#define TIFR2_OCF2A_MASK	0x1
#define TIFR2_TOV2_MASK	0x1

/**
 *	Timer/Counter 3 (16-bit)
 */
/* TCCR3A */
// register
#define TCCR3A			0x90

// bits
#define TCCR3A_COMA		6
#define TCCR3A_COMB		4
#define TCCR3A_WGM1		1
#define TCCR3A_WGM0		0

// masks
#define TCCR3A_COMA_MASK	0x3
#define TCCR3A_COMB_MASK	0x3
#define TCCR3A_WGM1_MASK	0x1
#define TCCR3A_WGM0_MASK	0x1

/* TCCR3B */
// register
#define TCCR3B			0x91

// bits
#define TCCR3B_ICNC		7
#define TCCR3B_ICES		6
#define TCCR3B_WGM3		4
#define TCCR3B_WGM2		3
#define TCCR3B_CS		0

// masks
#define TCCR3B_ICNC_MASK	0x1
#define TCCR3B_ICES_MASK	0x1
#define TCCR3B_WGM3_MASK	0x1
#define TCCR3B_WGM2_MASK	0x1
#define TCCR3B_CS_MASK	0x7

/* TCCR3C */
// register
#define TCCR3C			0x92

// bits
#define TCCR3C_FOCA		7
#define TCCR3C_FOCB		6

// masks
#define TCCR3C_FOCA_MASK	0x1
#define TCCR3C_FOCB_MASK	0x1

/* TCNT3L */
// register
#define TCNT3L			0x94

// bits
#define TCNT3L_CNTL		0

// masks
#define TCNT3L_CNTL_MASK	0xff

/* TCNT3H */
// register
#define TCNT3H			0x95

// bits
#define TCNT3H_CNTH		0

// masks
#define TCNT3H_CNTH_MASK	0xff

/* ICR3 */
// register
#define ICR3			0x96

/* OCR3A */
// register
#define OCR3A			0x98

/* OCR3B */
// register
#define OCR3B			0x9a

/* TIMSK3 */
// register
#define TIMSK3			0x71

// bits
#define TIMSK3_ICIE3		5
#define TIMSK3_OCIE3B		2
#define TIMSK3_OCIE3A		1
#define TIMSK3_TOIE3		0

// masks
#define TIMSK3_ICIE3_MASK	0x1
#define TIMSK3_OCIE3B_MASK	0x1
#define TIMSK3_OCIE3A_MASK	0x1
#define TIMSK3_TOIE3_MASK	0x1

/* TIFR3 */
// register
#define TIFR3			0x38

// bits
#define TIFR3_ICF3		5
#define TIFR3_OCF3B		2
#define TIFR3_OCF3A		1
#define TIFR3_TOV3		0

// masks
#define TIFR3_ICF3_MASK	0x1
#define TIFR3_OCF3B_MASK	0x1
#define TIFR3_OCF3A_MASK	0x1
#define TIFR3_TOV3_MASK	0x1

/**
 *	Analog Comparator and Digital Converter
 */
/* ADCL */
// register
#define ADCL			0x78

// bits
#define ADCL_DATAL		0

// masks
#define ADCL_DATAL_MASK	0xff

/* ADCH */
// register
#define ADCH			0x79

// bits
#define ADCH_DATAH		0

// masks
#define ADCH_DATAH_MASK	0xff

/* ADCSRA */
// register
#define ADCSRA			0x7a

// bits
#define ADCSRA_ADEN		7
#define ADCSRA_ADSC		6
#define ADCSRA_ADATE		5
#define ADCSRA_ADIF		4
#define ADCSRA_ADIE		3
#define ADCSRA_ADPS		0

// masks
#define ADCSRA_ADEN_MASK	0x1
#define ADCSRA_ADSC_MASK	0x1
#define ADCSRA_ADATE_MASK	0x1
#define ADCSRA_ADIF_MASK	0x1
#define ADCSRA_ADIE_MASK	0x1
#define ADCSRA_ADPS_MASK	0x7

/* ADCSRB */
// register
#define ADCSRB			0x7b

// bits
#define ADCSRB_ACME		6
#define ADCSRB_ADTS		0

// masks
#define ADCSRB_ACME_MASK	0x1
#define ADCSRB_ADTS_MASK	0x7

/* ADMUX */
// register
#define ADMUX			0x7c

// bits
#define ADMUX_REFS1		7
#define ADMUX_REFS0		6
#define ADMUX_ADLAR		5
#define ADMUX_MUX4		4
#define ADMUX_MUX3		3
#define ADMUX_MUX2		2
#define ADMUX_MUX1		1
#define ADMUX_MUX0		0

// masks
#define ADMUX_REFS1_MASK	0x1
#define ADMUX_REFS0_MASK	0x1
#define ADMUX_ADLAR_MASK	0x1
#define ADMUX_MUX4_MASK	0x1
#define ADMUX_MUX3_MASK	0x1
#define ADMUX_MUX2_MASK	0x1
#define ADMUX_MUX1_MASK	0x1
#define ADMUX_MUX0_MASK	0x1

/* DIDR0 */
// register
#define DIDR0			0x7e

// bits
#define DIDR0_ADC7D		7
#define DIDR0_ADC6D		6
#define DIDR0_ADC5D		5
#define DIDR0_ADC4D		4
#define DIDR0_ADC3D		3
#define DIDR0_ADC2D		2
#define DIDR0_ADC1D		1
#define DIDR0_ADC0D		0

// masks
#define DIDR0_ADC7D_MASK	0x1
#define DIDR0_ADC6D_MASK	0x1
#define DIDR0_ADC5D_MASK	0x1
#define DIDR0_ADC4D_MASK	0x1
#define DIDR0_ADC3D_MASK	0x1
#define DIDR0_ADC2D_MASK	0x1
#define DIDR0_ADC1D_MASK	0x1
#define DIDR0_ADC0D_MASK	0x1

/* DIDR1 */
// register
#define DIDR1			0x7f

// bits
#define DIDR1_AIN1D		1
#define DIDR1_AIN0D		0

// masks
#define DIDR1_AIN1D_MASK	0x1
#define DIDR1_AIN0D_MASK	0x1

/* ACSR */
// register
#define ACSR			0x50

// bits
#define ACSR_ACD		7
#define ACSR_ACBG		6
#define ACSR_ACO		5
#define ACSR_ACI		4
#define ACSR_ACIE		3
#define ACSR_ACIC		2
#define ACSR_ACIS		0

// masks
#define ACSR_ACD_MASK	0x1
#define ACSR_ACBG_MASK	0x1
#define ACSR_ACO_MASK	0x1
#define ACSR_ACI_MASK	0x1
#define ACSR_ACIE_MASK	0x1
#define ACSR_ACIC_MASK	0x1
#define ACSR_ACIS_MASK	0x3

/**
 *	Electrically Erasable Programmable Read-Only Memory (EEPROM)
 */
/* EECR */
// register
#define EECR			0x3f

// bits
#define EECR_EEPM1		5
#define EECR_EEPM0		4
#define EECR_EERIE		3
#define EECR_EEMPE		2
#define EECR_EEPE		1
#define EECR_EERE		0

// masks
#define EECR_EEPM1_MASK	0x1
#define EECR_EEPM0_MASK	0x1
#define EECR_EERIE_MASK	0x1
#define EECR_EEMPE_MASK	0x1
#define EECR_EEPE_MASK	0x1
#define EECR_EERE_MASK	0x1

/* EEDR */
// register
#define EEDR			0x40

// bits
#define EEDR_DATA		0

// masks
#define EEDR_DATA_MASK	0xff

/* EEARL */
// register
#define EEARL			0x41

// bits
#define EEARL_ADDRL		0

// masks
#define EEARL_ADDRL_MASK	0xff

/* EEARH */
// register
#define EEARH			0x42

// bits
#define EEARH_ADDRH		0

// masks
#define EEARH_ADDRH_MASK	0xf

/**
 *	Interrupt Vectors
 */
// number of interrupts
#define INT_VECTORS			35

// interrupt numbers
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


/**
 * Device Configuration
 */
#define UART_CNT			2
#define TIMER_CNT			4
#define PWM_CNT				6
#define I2C_CNT				1
#define SPI_CNT				3
#define WATCHDOG_CNT		1
#define ADC_CNT				8
#define EEPROM_CNT			1

#define FLASH_SIZE			_128k
#define SRAM_SIZE			_16k
#define EEPROM_SIZE			_4k

#define	WATCHDOG_HZ			_128k


#endif // ARCH_ATMEGA1284_H
