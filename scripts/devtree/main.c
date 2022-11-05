/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/list.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <parser.tab.h>
#include <export.h>
#include <options.h>
#include <nodes.h>


/* local/static prototypes */
static void complement_node(memory_node_t *node);


/* global functions */
int main(int argc, char **argv){
	FILE *fp;
	int r;


	opt_parse(argc, argv);

	/* parse device tree */
	if(nodes_init() != 0)
		goto err_0;

	if(devtreeparse(options.ifile_name) != 0)
		goto err_1;

	complement_node(memory_root());

	/* write output file */
	fp = stdout;

	if(options.ofile_name){
		printf("generating device tree export \"%s\"\n", options.ofile_name);
		fp = fopen(options.ofile_name, "w");
	}

	if(fp == 0x0){
		fprintf(stderr, "open \"%s\" failed \"%s\"\n", options.ofile_name, strerror(errno));
		goto err_1;
	}

	r = 0;

	switch(options.ofile_format){
	case FMT_C:
		r |= export_c_header(fp);

		if(options.export_sections & DT_DEVICES)	r |= export_device_c(device_root(), fp);
		if(options.export_sections & DT_MEMORY)		r |= export_memory_c(memory_root(), fp);
		break;

	case FMT_HEADER:
		r |= export_header_header(fp);

		if(options.export_sections & DT_DEVICES)	r |= export_device_header(device_root(), fp);
		if(options.export_sections & DT_MEMORY)		r |= export_memory_header(memory_root(), fp);
		break;

	case FMT_MAKE:
		r |= export_make_header(fp);

		if(options.export_sections & DT_DEVICES)	r |= export_device_make(device_root(), fp);
		if(options.export_sections & DT_MEMORY)		r |= export_memory_make(memory_root(), fp);
		break;
	}

	if(r != 0)
		goto err_2;

	nodes_cleanup();
	fclose(fp);

	return 0;


err_2:
	if(fp != stdout)
		unlink(argv[2]);

err_1:
	nodes_cleanup();

err_0:
	return 1;
}


/* local functions */
static void complement_node(memory_node_t *node){
	void *min = (void*)0xffffffff,
		 *max = 0x0;
	memory_node_t *child;


	if(node == 0x0 || list_empty(node->childs))
		return;

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
