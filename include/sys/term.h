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
	TIFL_CRNL = 0x1,
	TIFL_NLCR = 0x2,
} term_iflags_t;

typedef enum{
	TOFL_CRNL = 0x1,
	TOFL_NLCR = 0x2,
} term_oflags_t;

typedef enum{
	TLFL_ECHO = 0x1,
} term_lflags_t;

typedef struct{
	term_iflags_t iflags;
	term_oflags_t oflags;
	term_lflags_t lflags;
} term_flags_t;


#endif // SYS_TERM_H
