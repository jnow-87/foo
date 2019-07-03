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
#include <node.h>


/* local/static prototypes */
static void complement_node(memory_node_t *node);


/* global functions */
int main(int argc, char **argv){
	FILE *fp;
	int r;
	driver_node_t driver_root;
	memory_node_t memory_root;


	if(argc < 2){
		printf("usage: %s <device tree script> [<output file>]\n", argv[0]);
		return 1;
	}

	/* parse device tree */
	memset(&driver_root, 0x0, sizeof(driver_node_t));
	driver_root.name = "driver_root";
	driver_root.compatible = "";

	memset(&memory_root, 0x0, sizeof(memory_node_t));
	memory_root.name = "memory_root";

	if(devtreeparse(argv[1], &driver_root, &memory_root) != 0)
		return 2;

	/* complement node data */
	complement_node(&memory_root);

	/* write output file */
	fp = stdout;

	if(argc > 2)
		fp = fopen(argv[2], "w");

	if(fp == 0x0){
		fprintf(stderr, "open \"%s\" failed \"%s\"\n", argv[1], strerror(errno));
		return 3;
	}

	// header
	fprintf(fp,
		"#ifdef BUILD_HOST\n"
		"#include <stdint.h>\n"
		"#else\n"
		"#include <sys/types.h>\n"
		"#endif // BUILD_HOST\n"
		"\n"
		"#include <sys/devtree.h>\n"
		"\n"
		"\n"
	);

	// device tree
	r = 0;

	r |= node_export_driver(&driver_root, fp);
	r |= node_export_memory(&memory_root, fp);

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

	if(node->size == 0 && strcmp(node->name, "memory_root") != 0){
		node->base = min;
		node->size = max - min;
	}
}


