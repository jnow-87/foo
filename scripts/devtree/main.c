/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <parser.tab.h>
#include <export.h>
#include <nodes.h>
#include <options.h>


/* local/static prototypes */
static FILE *output_file(char const *file);


/* global functions */
int main(int argc, char **argv){
	FILE *ofile;


	opt_parse(argc, argv);

	/* parse device tree */
	if(nodes_init() != 0)
		return 1;

	if(devtreeparse(options.ifile_name) != 0)
		return 1;

	/* write output file */
	ofile = output_file(options.ofile_name);

	if(ofile == 0x0)
		return 1;

	switch(options.ofile_format){
	case FMT_HEADER:	export_header(ofile); break;
	case FMT_C:			export_source(ofile); break;
	case FMT_MAKE:		export_make(ofile); break;
	}

	if(options.ofile_name != 0x0)
		fclose(ofile);

	return 0;
}


/* local functions */
static FILE *output_file(char const *file){
	FILE *fp;


	if(file == 0x0)
		return stdout;

	printf("generating device tree export \"%s\"\n", options.ofile_name);

	fp = fopen(options.ofile_name, "w");

	if(fp == 0x0){
		fprintf(stderr, "open \"%s\" failed \"%s\"\n", options.ofile_name, strerror(errno));

		return 0x0;
	}

	return fp;
}
