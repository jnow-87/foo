#ifndef SYS_UART_H
#define SYS_UART_H


#include <sys/types.h>


/* types */
typedef enum{
	BR_2400 = 2400,
	BR_4800 = 4800,
	BR_9600 = 9600,
	BR_14400 = 14400,
	BR_19200 = 19200,
	BR_28800 = 28800,
	BR_38400 = 38400,
	BR_57600 = 57600,
	BR_76800 = 76800,
	BR_115200 = 115200,
	BR_230400 = 230400,
	BR_250000 = 250000,
	BR_500000 = 500000,
	BR_1000000 = 1000000,
} baudrate_t;

typedef enum{
	STOPB1 = 0,
	STOPB2,
} stopb_t;

typedef enum{
	PARITY_NONE = 0,
	PARITY_ODD,
	PARITY_EVEN,
} parity_t;

typedef enum{
	CS_5 = 0,
	CS_6,
	CS_7,
	CS_8,
} csize_t;

typedef struct{
	baudrate_t baud;
	stopb_t stopb;
	parity_t parity;
	csize_t csize;

	bool data_overrun,
		 parity_err,
		 frame_err,
		 rx_queue_full;
} uart_t;


#endif // SYS_UART_H
