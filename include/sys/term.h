/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_TERM_H
#define SYS_TERM_H


/* types */
typedef enum{
	TERM_ERR_NONE = 0x0,
	TERM_ERR_DATA_OVERRUN = 0x1,
	TERM_ERR_PARITY = 0x2,
	TERM_ERR_FRAME = 0x4,
	TERM_ERR_RX_FULL = 0x8,
	TERM_ERR_WRITE_COLL = 0x10,
} term_err_t;

typedef enum{
	TERM_FLAG_ECHO = 0x1,
} term_flags_t;


#endif // SYS_TERM_H
