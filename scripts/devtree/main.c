/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <parser.tab.h>
#include <export.h>
#include <options.h>


/* local/static prototypes */
static void complement_node(memory_node_t *node);


/* global functions */
int main(int argc, char **argv){
	FILE *fp;
	int r;
	driver_node_t driver_root;
	memory_node_t memory_root;


	/* parse arguments */
	opt_parse(argc, argv);

	/* parse device tree */
	memset(&driver_root, 0, sizeof(driver_node_t));
	driver_root.name = "driver_root";
	driver_root.compatible = "";

	memset(&memory_root, 0, sizeof(memory_node_t));
	memory_root.name = "memory_root";

	if(devtreeparse(options.ifile_name, &driver_root, &memory_root) != 0)
		return 2;

	/* complement node data */
	complement_node(&memory_root);

	/* write output file */
	fp = stdout;

	if(options.ofile_name){
		printf("generating device tree export \"%s\"\n", options.ofile_name);
		fp = fopen(options.ofile_name, "w");
	}

	if(fp == 0x0){
		fprintf(stderr, "open \"%s\" failed \"%s\"\n", options.ofile_name, strerror(errno));
		return 3;
	}


	r = 0;

	switch(options.ofile_format){
	case FMT_C:
		r |= export_c_header(fp);

		if(options.export_sections & DT_DRIVER)	r |= export_driver_c(&driver_root, fp);
		if(options.export_sections & DT_MEMORY)	r |= export_memory_c(&memory_root, fp);
		break;

	case FMT_HEADER:
		r |= export_header_header(fp);

		if(options.export_sections & DT_DRIVER)	r |= export_driver_header(&driver_root, fp);
		if(options.export_sections & DT_MEMORY)	r |= export_memory_header(&memory_root, fp);
		break;

	case FMT_MAKE:
		r |= export_make_header(fp);

		if(options.export_sections & DT_DRIVER)	r |= export_driver_make(&driver_root, fp);
		if(options.export_sections & DT_MEMORY)	r |= export_memory_make(&memory_root, fp);
		break;
	}

	fclose(fp);

	if(r != 0 && fp != stdout){
		unlink(argv[2]);
		return 4;
	}

	return 0;
}


/* local functions */
static void complement_node(memory_node_t *node){
	void *min,
		 *max;
	memory_node_t *child;


	if(node == 0x0 || list_empty(node->childs))
		return;

	min = (void*)0xffffffff;
	max = 0x0;

	list_for_each(node->childs, child){
		complement_node(child);

		if(min > child->base)
			min = child->base;

		if(max < child->base + child->size)
			max = child->base + child->size;
	}

	if(node->size == 0){
		node->base = min;
		node->size = max - min;
	}
}
