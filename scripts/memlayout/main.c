/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/compiler.h>
#include <sys/devtree.h>
#include <sys/escape.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "config.h"


/* macros */
#define ERROR(msg, ...) ({ fprintf(stderr, FG("error", RED) ": " msg, __VA_ARGS__); 1; })

#define CONTAINS(n0, n1)({ \
	typeof(n0) _n0 = n0; \
	typeof(n1) _n1 = n1; \
	\
	\
	(_n0->base <= _n1->base && _n1->base + _n1->size <= _n0->base + _n0->size); \
})

#define OVERLAPS(n0, n1)({ \
	typeof(n0) _n0 = n0; \
	typeof(n1) _n1 = n1; \
	\
	\
	(_n0->base < _n1->base + _n1->size && _n1->base < _n0->base + _n0->size); \
})



/* local/static prototypes */
static int print(devtree_node_t const *node, size_t level, void *data);

static int check_availability(void);
static int check_child_is_memory(devtree_node_t const *node, size_t level, void *data);
static int check_bounds(devtree_node_t const *node, size_t level, void *data);
static int check_overlap(devtree_node_t const *node, size_t level, void *data);


/* global functions */
int main(int argc, char **argv){
	int r = 0;


	if(argc == 1){
		devtree_traverse(print, 0x0, 0x0);
	}
	else if(strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--check") == 0){
		r |= check_availability();

		devtree_traverse(check_child_is_memory, devtree_filter_memory, &r);

		if(r == 0){
			devtree_traverse(check_bounds, devtree_filter_memory, &r);
			devtree_traverse(check_overlap, devtree_filter_memory, &r);
		}
	}
	else{
		printf("invalid option \n");
		printf("usage: %s [-c|--check]\n", argv[0]);

		r = 1;
	}

	return r != 0;
}


/* local functions */
/**
 * \brief	print the memory nodes defined in the device tree
 */
static int print(devtree_node_t const *node, size_t level, void *data){
	static size_t top_level = 0;
	size_t name_len = strlen(node->name);
	char name[name_len + 1];
	devtree_memory_payload_t const *pl;


	if(level == 0)
		top_level = 0;

	if(strcmp(node->compatible, "memory") != 0)
		return 0;

	if(top_level == 0)
		top_level = level;

	strcpy(name, node->name);

	for(size_t i=0; i<name_len; i++){
		if(name[i] == '_')
			name[i] = ' ';
	}

	pl = node->payload;

	if(level == top_level){
		printf("\n" ATTR(BG(FG("%s:", BLACK), WHITE), UNDERLINE) "\n" BG(FG("         target         base          end        size               ", BLACK), WHITE) "\n", name);

		if(pl->size != 0)
			printf(ATTR("%20.20s", BOLD) ": 0x%8.8x - 0x%8.8x %8.2fk (%#8.8x)\n\n", "total", pl->base, pl->base + pl->size - 1, pl->size / 1024.0, pl->size);
	}
	else
		printf(ATTR("%20.20s", BOLD) ": 0x%8.8x - 0x%8.8x %8.2fk (%#8.8x)\n", name, pl->base, pl->base + pl->size - 1, pl->size / 1024.0, pl->size);

	return 0;
}

/**
 * \brief	check if all sections defined in required_nodes are present in the
 * 			device tree
 */
static int check_availability(void){
	int r = 0;


	for(size_t i=0; i<sizeof_array(required_nodes); i++){
		if(devtree_find("memory", required_nodes[i]) == 0x0){
			ERROR("device tree lacks node \"%s\"\n", required_nodes[i]);
			r++;
		}
	}

	return -r;
}


/**
 * \brief	check that all child nodes of a memory node are also memory nodes
 */
static int check_child_is_memory(devtree_node_t const *node, size_t level, void *data){
	int r = 0;
	devtree_node_t const *child;


	if(node->childs == 0x0)
		return 0;

	for(size_t i=0; node->childs[i]!=0x0; i++){
		child = node->childs[i];

		if(devtree_filter_memory(child) != 0)
			r |= ERROR("child node \"%s\" of memory node \"%s\" is not a memory node\n", child->name, node->name);
	}

	*((int*)data) |= r;

	return 0;
}

/**
 * \brief	check if all address ranges of child nodes are within the range of
 * 			the parent node
 */
static int check_bounds(devtree_node_t const *node, size_t level, void *data){
	int r = 0;
	devtree_memory_payload_t const *pl = node->payload;
	devtree_node_t const *child;


	if(node->childs == 0x0 || pl->size == 0)
		return 0;

	for(size_t i=0; node->childs[i]!=0x0; i++){
		child = node->childs[i];

		if(!CONTAINS(pl, (devtree_memory_payload_t*)child->payload))
			r |= ERROR("memory node \"%s\" address range is outside of parent node \"%s\"\n", child->name, node->name);
	}

	*((int*)data) |= r;

	return 0;
}


/**
 * \brief	check if sibling nodes have overlapping address ranges
 */
static int check_overlap(devtree_node_t const *node, size_t level, void *data){
	int r = 0;
	devtree_memory_payload_t const *pl0,
								   *pl1;


	if(node->childs == 0x0)
		return 0;

	for(size_t i=0; node->childs[i]!=0x0; i++){
		pl0 = node->childs[i]->payload;

		for(size_t j=i+1; node->childs[j]!=0x0; j++){
			pl1 = node->childs[j]->payload;

			if(pl0->size == 0 || pl1->size == 0)
				continue;

			if(OVERLAPS(pl0, pl1))
				r |= ERROR("memory nodes \"%s\" and \"%s\" have overlapping address ranges\n", node->childs[i]->name, node->childs[j]->name);
		}
	}

	*((int*)data) |= r;

	return 0;
}
