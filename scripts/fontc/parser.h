/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef FONTC_PARSER_H
#define FONTC_PARSER_H


#include <stdio.h>


/* types */
typedef struct{
	char name[64];

	size_t first_char,
		   last_char,
		   height,
		   width;
} font_header_t;


/* prototypes */
int parse_header(FILE *fp, font_header_t *hdr);
int parse_letter(FILE *fp, char *letter, font_header_t *hdr);


#endif // FONTC_PARSER_H
