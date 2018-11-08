/*
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_TERM_H
#define SYS_TERM_H


#include <sys/types.h>


/* types */
typedef enum{
	TBR_2400 = 2400,
	TBR_4800 = 4800,
	TBR_9600 = 9600,
	TBR_14400 = 14400,
	TBR_19200 = 19200,
	TBR_28800 = 28800,
	TBR_38400 = 38400,
	TBR_57600 = 57600,
	TBR_76800 = 76800,
	TBR_115200 = 115200,
	TBR_230400 = 230400,
	TBR_250000 = 250000,
	TBR_500000 = 500000,
	TBR_1000000 = 1000000,
} term_baudrate_t;

typedef enum{
	TSTOPB1 = 0,
	TSTOPB2,
} term_stopb_t;

typedef enum{
	TPARITY_NONE = 0,
	TPARITY_ODD,
	TPARITY_EVEN,
} term_parity_t;

typedef enum{
	TCS_5 = 0,
	TCS_6,
	TCS_7,
	TCS_8,
} term_csize_t;

typedef enum{
	TF_ECHO = 0x1,
} term_flags_t;

typedef enum{
	TE_NONE = 0x0,
	TE_DATA_OVERRUN = 0x1,
	TE_PARITY = 0x2,
	TE_FRAME = 0x4,
	TE_RX_FULL = 0x8,
} term_err_t;

typedef struct{
	term_baudrate_t baud;
	term_stopb_t stopb;
	term_parity_t parity;
	term_csize_t csize;
	term_flags_t flags;
} term_cfg_t;


#endif // SYS_TERM_H
