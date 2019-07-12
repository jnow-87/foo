/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_UART_H
#define SYS_UART_H


#include <sys/term.h>
#include <sys/types.h>


/* types */
typedef enum{
	UART_BR_0 = 0,
	UART_BR_2400 = 2400,
	UART_BR_4800 = 4800,
	UART_BR_9600 = 9600,
	UART_BR_14400 = 14400,
	UART_BR_19200 = 19200,
	UART_BR_28800 = 28800,
	UART_BR_38400 = 38400,
	UART_BR_57600 = 57600,
	UART_BR_76800 = 76800,
	UART_BR_115200 = 115200,
	UART_BR_230400 = 230400,
	UART_BR_250000 = 250000,
	UART_BR_500000 = 500000,
	UART_BR_1000000 = 1000000,
} uart_baudrate_t;

typedef enum{
	UART_STOPB1 = 0,
	UART_STOPB2,
} uart_stopb_t;

typedef enum{
	UART_PARITY_NONE = 0,
	UART_PARITY_ODD,
	UART_PARITY_EVEN,
} uart_parity_t;

typedef enum{
	UART_CS_5 = 0,
	UART_CS_6,
	UART_CS_7,
	UART_CS_8,
} uart_csize_t;

typedef struct{
	/**
	 * NOTE fixed-size types are used to allow
	 * 		using this type with the device tree
	 */

	uint32_t baudrate;	/**< cf. uart_baudrate_t */
	uint8_t stopb;		/**< cf. uart_stopb_t */
	uint8_t parity;		/**< cf. uart_parity_t */
	uint8_t csize;		/**< cf. uart_csize_t */

	uint8_t flags;		/**< cf. term_flags_t */
} uart_cfg_t;


#endif // SYS_UART_H
