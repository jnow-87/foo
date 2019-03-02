/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/list.h>
#include <sys/vector.h>
#include <sys/escape.h>
#include <parser.tab.h>
#include <node.h>
#include <stdio.h>


/* macros */
#define ERROR(fmt, ...) \
	printf(FG_RED "error" RESET_ATTR ": " fmt, ##__VA_ARGS__)


/* global functions */
int main(int argc, char **argv){
	node_t root;


	memset(&root, 0x0, sizeof(node_t));
	root.name = "root";
	root.compatible = "root";

	if(devtreeparse(argv[1], &root) != 0)
		return 1;

	node_print(&root, 0);

	return 0;
}
