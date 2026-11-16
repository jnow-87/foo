/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



/* target header */
#include <config/config.h>
#include <sys/devtree.h>
#include <sys/escape.h>
#include <sys/string.h>

/* host header */
#include <stdio.h>
#include <errno.h>


/* local/static prototypes */
static int export(devtree_node_t const *node, size_t level, void *data);


/* global functions */
int main(int argc, char **argv){
	char *ofile_name;
	FILE *ofile;


	/* check arguments */
	if(argc < 2){
		printf("usage: %s <output>\n\n"
			   "%15s    %s\n"
			   ,
			   argv[0],
			   "<output>", "output linker file name"
		);

		return 1;
	}

	ofile_name = argv[1];

	printf("generating memory linker script \"%s\"\n", ofile_name);

	/* generate output file */
	ofile = fopen(ofile_name, "w");

	if(ofile == 0){
		printf("open output file failed \"%s\"\n", strerror(errno));
		return 1;
	}

	fprintf(ofile, "MEMORY {\n");
	devtree_traverse(export, devtree_filter_memory, ofile);
	fprintf(ofile, "}\n");

	fclose(ofile);

	return 0;
}


/* local functions */
static int export(devtree_node_t const *node, size_t level, void *data){
	devtree_memory_payload_t const *pl = node->payload;


	fprintf(data, "%20s : ORIGIN = %#10lx, LENGTH = %u\n", strcidtfr(node->name), (unsigned long int)pl->base, pl->size);

	return 0;
}
