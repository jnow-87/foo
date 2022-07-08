/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef FONTC_OPTS_H
#define FONTC_OPTS_H


#include <stdbool.h>


/* types */
typedef struct{
	char const *font,
			   *source;

	bool vertical;
} opts_t;


/* prototypes */
int opts_parse(int argc, char **argv);


/* external variables */
extern opts_t opts;


#endif // FONTC_OPTS_H
