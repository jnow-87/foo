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
#define ERROR(msg, ...) fprintf(stderr, FG("error", RED) ": " msg, __VA_ARGS__)

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
static void print_layout(devtree_memory_t const *node, bool top_level);

static int check_availability(void);
static int check_bounds(devtree_memory_t const *node);
static int check_overlap(devtree_memory_t const *node);


/* global functions */
int main(int argc, char **argv){
	int r = 0;


	if(argc == 1){
		print_layout(&__dt_memory_root, false);
	}
	else if(strcmp(argv[1], "-c") == 0 || strcmp(argv[1], "--check") == 0){
		r |= check_availability();
		r |= check_bounds(&__dt_memory_root);
		r |= check_overlap(&__dt_memory_root);
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
static void print_layout(devtree_memory_t const *node, bool top_level){
	char name[node != 0x0 ? strlen(node->name) : 1];


	if(node == 0x0)
		return;

	if(node != &__dt_memory_root){
		strcpy(name, node->name);

		for(size_t i=0; i<strlen(name); i++){
			if(name[i] == '_')
				name[i] = ' ';
		}

		if(top_level){
			printf("\n" ATTR(BG(FG("%s:", BLACK), WHITE), UNDERLINE) "\n" BG(FG("         target         base          end        size               ", BLACK), WHITE) "\n", name);

			if(node->size != 0)
				printf(ATTR("%20.20s", BOLD) ": 0x%8.8x - 0x%8.8x %8.2fk (%#8.8x)\n\n", "total", node->base, node->base + node->size - 1, node->size / 1024.0, node->size);
		}
		else
			printf(ATTR("%20.20s", BOLD) ": 0x%8.8x - 0x%8.8x %8.2fk (%#8.8x)\n", name, node->base, node->base + node->size - 1, node->size / 1024.0, node->size);
	}

	if(node->childs == 0x0)
		return;

	for(size_t i=0; node->childs[i]!=0x0; i++)
		print_layout(node->childs[i], (node == &__dt_memory_root));
}

/**
 * \brief	check if all sections defined in required_nodes are present in the
 * 			device tree
 */
static int check_availability(void){
	int r = 0;


	for(size_t i=0; i<sizeof_array(required_nodes); i++){
		if(devtree_find_memory_by_name(&__dt_memory_root, required_nodes[i]) == 0x0){
			ERROR("device tree lacks node \"%s\"\n", required_nodes[i]);
			r++;
		}
	}

	return -r;
}

/**
 * \brief	check if all address ranges of child nodes are within the range of
 * 			the parent node
 */
static int check_bounds(devtree_memory_t const *node){
	int r = 0;


	if(node == 0x0 || node->childs == 0x0)
		return 0;

	for(size_t i=0; node->childs[i]!=0x0; i++){
		r -= check_bounds(node->childs[i]);

		if(node->size == 0)
			continue;

		if(!CONTAINS(node, node->childs[i])){
			ERROR("memory node \"%s\" address range is outside of parent node \"%s\"\n", node->childs[i]->name, node->name);
			r++;
		}
	}

	return -r;
}


/**
 * \brief	check if sibling nodes have overlapping address ranges
 */
static int check_overlap(devtree_memory_t const *node){
	int r = 0;


	if(node == 0x0 || node->childs == 0x0)
		return 0;

	for(size_t i=0; node->childs[i]!=0x0; i++){
		r -= check_overlap(node->childs[i]);

		for(size_t j=i+1; node->childs[j]!=0x0; j++){
			if(node->childs[i]->size == 0 || node->childs[j]->size == 0)
				continue;

			if(OVERLAPS(node->childs[i], node->childs[j])){
				ERROR("memory node \"%s\" has overlapping address range with node \"%s\"\n", node->childs[i]->name, node->childs[j]->name);
				r++;
			}
		}
	}

	return -r;
}
