/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include "opts.h"
#include "user.h"
#include "parser.h"
#include "compiler.h"


/* local/static prototypes */
static FILE *open(char const *file, char const *mode);


/* global functions */
int main(int argc, char **argv){
	FILE *fp_font,
		 *fp_src;
	font_header_t hdr;


	/* parse command line arguments */
	if(opts_parse(argc, argv) != 0)
		return 1;

	fp_font = open(opts.font, "r");
	fp_src = open(opts.source, "w");

	if(fp_font == 0x0 || fp_src == 0x0)
		goto err;

	/* parse font header */
	memset(&hdr, 0x0, sizeof(font_header_t));

	if(parse_header(fp_font, &hdr) != 0)
		goto err;

	/* compile */
	printf("generating font source \"%s\"\n", opts.source);

	if(compile_font(fp_font, fp_src, &hdr) != 0)
		goto err;

	/* cleanup */
	if(fp_src)
		fclose(fp_src);

	fclose(fp_font);

	return 0;


err:
	if(fp_font)
		fclose(fp_font);

	if(fp_src)
		fclose(fp_src);

	unlink(opts.source);

	return 1;
}


/* local functions */
static FILE *open(char const *file, char const *mode){
	FILE *fp;


	if(file == 0x0)
		return 0x0;

	fp = fopen(file, mode);

	if(fp == 0x0)
		ERROR("opening %s\n", file);

	return fp;
}
