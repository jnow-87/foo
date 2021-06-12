/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_LOOP_H
#define SYS_LOOP_H


#include <sys/term.h>
#include <sys/types.h>


/* types */
typedef struct{
	/**
	 * NOTE fixed-size types are used to allow
	 * 		using this type with the device tree
	 */

	uint8_t iflags,		/**< cf. term_flags_t */
			oflags,
			lflags;
} loop_cfg_t;


#endif // SYS_LOOP_H
