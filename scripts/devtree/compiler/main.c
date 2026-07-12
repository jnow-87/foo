/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/vector.h>
#include <parser.tab.h>
#include "codegen.h"
#include "node.h"
#include "opt.h"


/* local/static prototypes */
static int collect_nodes(vector_t *nodes);
static FILE *output_file(char const *file);


// TODO check memory allocation, especially free
// TODO overall review
// TODO extend parser to support gcc preproc line information
// 		update scripts/build/compile.make::preproc_file, removing -P to get them
//
// TODO simplify ternary operators via bool casts as done in 1e177e4fac19aabf2ecec6424231105d047a4875


/* global functions */
int main(int argc, char **argv){
	int r = -1;
	FILE *ofile;
	vector_t nodes = VECTOR_INITIALISER(sizeof(node_t*));


	opt_parse(argc, argv);

	if(nodes_init() != 0)
		goto end;

	/* parse device tree */
	if(devtreeparse(options.ifile_name) != 0)
		goto end;

	printf("parsed\n");
	return 0;

	if(nodes_assert() != 0)
		goto end;

	/* write output file */
	if(collect_nodes(&nodes) != 0)
		goto end;

	ofile = output_file(options.ofile_name);

	switch(options.ofile_format){
	case FMT_HEADER:	codegen_header(ofile, &nodes); break;
	case FMT_C:			codegen_source(ofile, &nodes); break;
	case FMT_MAKE:		codegen_make(ofile, &nodes); break;
	}

	if(options.ofile_name != 0x0)
		fclose(ofile);

	r = 0;

end:
	vector_destroy(&nodes);

	return -r;
}


/* local functions */
static int collect_nodes(vector_t *nodes){
	int r = 0;
	char *tk;
	node_t *node;


	if(options.nodes != 0x0){
		while((tk = strtok(options.nodes, ","))){
			options.nodes = 0x0;
			node = node_ref(tk);

			if(node == 0x0 || vector_add(nodes, &node) != 0)
				return -1;
		}
	}
	else
		r |= vector_add(nodes, &(node_t*){ nodes_root() });

	return r;
}

static FILE *output_file(char const *file){
	FILE *fp;


	if(file == 0x0)
		return stdout;

	fprintf(stderr, "generating device tree export \"%s\"\n", options.ofile_name);

	fp = fopen(options.ofile_name, "w");

	if(fp == 0x0){
		fprintf(stderr, "open \"%s\" failed \"%s\"\n", options.ofile_name, strerror(errno));

		return 0x0;
	}

	return fp;
}
