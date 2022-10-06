/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_TERM_H
#define SYS_TERM_H


#include <sys/types.h>
#include <sys/uart.h>
#include <sys/spi.h>
#include <sys/loop.h>
#include <sys/vram.h>


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
	TLFL_CANON = 0x2,
	TLFL_WRAP = 0x4,
	TLFL_SCROLL = 0x8,
	TLFL_CURSOR = 0x10,
} term_lflags_t;

typedef struct{
	/**
	 * NOTE fixed-size types are used to allow
	 * 		using this type with the device tree
	 */

	uint8_t iflags;	/**< cf. term_iflags_t */
	uint8_t oflags;	/**< cf. term_oflags_t */
	uint8_t lflags;	/**< cf. term_lflags_t */

	uint8_t tabs;
	uint16_t lines,
			 columns;
} term_cfg_t;

typedef struct{
	term_cfg_t term;
	uart_cfg_t uart;
} term_uart_cfg_t;

typedef struct{
	term_cfg_t term;
	spi_cfg_t spi;
} term_spi_cfg_t;

typedef struct{
	term_cfg_t term;
	loop_cfg_t loop;
} term_loop_cfg_t;

typedef struct{
	term_cfg_t term;
	vram_cfg_t vram;
} term_vram_cfg_t;


#endif // SYS_TERM_H
