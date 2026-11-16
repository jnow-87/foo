/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <sys/devtree.h>
#include <sys/escape.h>
#include <sys/string.h>
#include <sys/types.h>


/* local/static prototypes */
static int print(devtree_node_t const *node, size_t level, void *data);
static void print_memory(devtree_node_t const *node, size_t indent);
static void print_device(devtree_node_t const *node, size_t indent);


/* global functions */
int main(int argc, char **argv){
	devtree_traverse(print, 0x0, 0x0);

	return 0;
}


/* local functions */
static int print(devtree_node_t const *node, size_t level, void *data){
	if(strcmp(node->compatible, "memory") == 0)	print_memory(node, level * 2);
	else										print_device(node, level * 2);

	return 0;
}

static void print_memory(devtree_node_t const *node, size_t indent){
	devtree_memory_payload_t *pl = (devtree_memory_payload_t*)node->payload;


	printf("%*s" FG("%s", KOBALT) " (base=%#x, end=%#x, size=%.2fk)\n"
		, indent, ""
		, node->name
		, pl->base
		, pl->base + pl->size
		, (float)pl->size / 1024
	);
}

static void print_device(devtree_node_t const *node, size_t indent){
	printf("%*s" FG("%s", KOBALT), indent, "", node->name);

	if(node->compatible != 0x0 && node->compatible[0] != 0)
		printf(" (driver=%s)\n", node->compatible);

	printf("\n");
}
