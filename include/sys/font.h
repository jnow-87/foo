/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_FONT_H
#define SYS_FONT_H


#include <sys/types.h>


/* types */
typedef struct{
	char const *name;

	uint8_t first_char,
			last_char,
			height,
			width;

	uint8_t *data[];
} font_t;


#endif // SYS_FONT_H
