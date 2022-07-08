/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef FONTC_COMPILER_H
#define FONTC_COMPILER_H


#include <stdio.h>
#include "parser.h"


/* prototypes */
int compile_font(FILE *ifp, FILE *ofp, font_header_t *hdr);


#endif // FONTC_COMPILER_H
