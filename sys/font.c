/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/types.h>
#include <sys/string.h>
#include <sys/font.h>


/* external variables */
extern font_t __start_fonts[],
			  __stop_fonts[];


/* global functions */
font_t *font_resolve(char const *name){
	if(name == 0x0 || strlen(name) == 0)
		return __start_fonts;

	for(font_t *font=__start_fonts; font!=__stop_fonts; font++){
		if(strcmp(name, font->name) == 0)
			return font;
	}

	return __start_fonts;
}

uint8_t *font_char(char c, font_t *font){
	if(c < font->first_char || c > font->last_char)
		return font->chars[0];

	return font->chars[c - font->first_char];
}
