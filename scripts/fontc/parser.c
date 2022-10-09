/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "user.h"
#include "parser.h"


/* macros */
#define HEADER_DELIMITER	"%%"


/* local/static prototypes */
static int sanitise_header(font_header_t *hdr);
static int gettoken(FILE *fp, char *tk, size_t n);


/* global functions */
int parse_header(FILE *fp, font_header_t *hdr){
	size_t i;
	char tk[3][64];


	hdr->name[0] = 0;

	while(1){
		/* read tokens */
		for(i=0; i<3; i++){
			if(gettoken(fp, tk[i], sizeof(tk[0])) != 0)
				return -1;

			if(strcmp(tk[i], HEADER_DELIMITER) == 0)
				return sanitise_header(hdr);
		}

		/* check for "<key> = <value>" */
		if(strcmp(tk[1], "=") != 0)
			return ERROR("invalid token %s, expected '='\n", tk[1]);

		/* update font header */
		if(strcmp(tk[0], "name") == 0){
			if(strlen(tk[2]) >= sizeof(hdr->name))
				return ERROR("name longer than %zu\n", sizeof(hdr->name));

			strcpy(hdr->name, tk[2]);
		}
		else if(strcmp(tk[0], "height") == 0){
			hdr->height = atoi(tk[2]);
		}
		else if(strcmp(tk[0], "width") == 0){
			hdr->width = atoi(tk[2]);
		}
		else if(strcmp(tk[0], "first_char") == 0){
			hdr->first_char = atoi(tk[2]);
		}
		else if(strcmp(tk[0], "last_char") == 0){
			hdr->last_char = atoi(tk[2]);
		}
		else
			return ERROR("invalid token %s\n", tk[0]);
	}
}

int parse_letter(FILE *fp, char *letter, font_header_t *hdr){
	char c;
	size_t i,
		   j;

	for(i=0; i<hdr->height; i++){
		j = 0;

		while(j < hdr->width){
			if(fread(&c, 1, 1, fp) != 1)
				return (i != 0 || j != 0) ? ERROR("insufficient letter data\n") : 0;

			if(c == '\n' || c == '\r'){
				if(j == 0)
					continue;

				return ERROR("letter with invalid line length\n");
			}

			letter[i * hdr->width + j++] = c;
		}
	}

	return 0;
}


/* local functions */
static int sanitise_header(font_header_t *hdr){
	if(hdr->name[0] == 0)
		return ERROR("font name not defined\n");

	if(hdr->height == 0 || hdr->height > 8)
		return ERROR("invalid height %zu, expect [1..8]\n", hdr->height);

	if(hdr->width == 0)
		return ERROR("invalid height %zu, expect > 0\n", hdr->width);

	if(hdr->first_char == 0 || hdr->last_char == 0)
		return ERROR("invalid first-/last- character\n");

	return 0;
}

static int gettoken(FILE *fp, char *tk, size_t n){
	char c;
	size_t i;


	i = 0;

	while(i < n){
		if(fread(&c, 1, 1, fp) != 1)
			return ERROR("reading token\n");

		if(isblank(c) || c == '\n' || c == '\r'){
			if(i == 0)
				continue;

			tk[i] = 0;

			return 0;
		}

		if(isalnum(c) || c == '_' || c == '=' || c == '%'){
			tk[i++] = c;
		}
		else
			return ERROR("invalid character '%c'\n", c);
	}

	return ERROR("token too long\n");
}
