/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DRIVER_TERM_H
#define DRIVER_TERM_H


#include <arch/interrupt.h>
#include <sys/term.h>
#include <sys/types.h>


/* types */
typedef struct{
	int (*configure)(void *cfg, void *regs);

	term_flags_t (*get_flags)(void *cfg);

	void (*putc)(char c, void *regs);
	size_t (*puts)(char const *s, size_t n, void *regs);
	size_t (*gets)(char *s, size_t n, term_err_t *err, void *regs);

	void *regs;
	int_num_t rx_int,
			  tx_int;

	uint8_t cfg_size;
} term_itf_t;


#endif // DRIVER_TERM_H
