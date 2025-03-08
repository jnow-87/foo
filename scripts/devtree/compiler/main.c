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
#include <export.h>
#include <nodes.h>
#include <options.h>
#include <parser.tab.h>


/* local/static prototypes */
static int collect_nodes(vector_t *nodes);
static FILE *output_file(char const *file);


// TODO
// gfe -Wg,-i core_mask
// gfe -Wg,-i arch_multi_core


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

	/* write output file */
	if(collect_nodes(&nodes) != 0)
		goto end;

	ofile = output_file(options.ofile_name);

	switch(options.ofile_format){
	case FMT_HEADER:	export_header(ofile, &nodes); break;
	case FMT_C:			export_source(ofile, &nodes); break;
	case FMT_MAKE:		export_make(ofile, &nodes); break;
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
	else{
		r |= vector_add(nodes, &(node_t*){ nodes_root(TC_MEMORY) });
		r |= vector_add(nodes, &(node_t*){ nodes_root(TC_DEVICE) });
	}

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
