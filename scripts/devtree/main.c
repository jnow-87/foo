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


/* global functions */
int main(int argc, char **argv){
	FILE *fp;
	int r;
	node_t root;


	if(argc < 2){
		printf("usage: %s <device tree script> [<output file>]\n", argv[0]);
		return 1;
	}

	/* parse device tree */
	memset(&root, 0x0, sizeof(node_t));
	root.name = "root";
	root.compatible = "";

	if(devtreeparse(argv[1], &root) != 0)
		return 2;

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
		"#include <kernel/devtree.h>\n"
		"\n"
		"\n"
	);

	// device tree
	r = node_export(&root, fp);

	fclose(fp);

	if(r != 0 && fp != stdout){
		unlink(argv[2]);
		return 4;
	}

	return 0;
}
