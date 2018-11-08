/*
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_BINLOADER_H
#define SYS_BINLOADER_H


/* types */
typedef enum{
	BIN_RAW = 0x0,
	BIN_ELF,
	NBINLOADER
} bin_type_t;


#endif // SYS_BINLOADER_H
