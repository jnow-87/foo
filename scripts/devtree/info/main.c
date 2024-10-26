/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/devtree.h>
#include <sys/escape.h>
#include <sys/types.h>
#include <stdio.h>


/* local/static prototypes */
static void print_arch(void);
static void print_memory(devtree_memory_t const *node, size_t indent, char const *colour);
static void print_device(devtree_device_t const *node, size_t indent, char const *colour);


/* global functions */
int main(int argc, char **argv){
	print_arch();
	printf("\n");
	print_memory(&__dt_memory_root, 0, FG_PURPLE);
	printf("\n");
	print_device(&__dt_device_root, 0, FG_PURPLE);

	return 0;
}


/* local functions */
static void print_arch(void){
	devtree_arch_t const *node = &__dt_arch_root;


	printf(FG("arch_root", PURPLE) "\n");

	if(node->childs == NULL)
		return;

	for(size_t i=0; node->childs[i]!=NULL; i++)
		print_device(node->childs[i], 2, FG_KOBALT);
}

static void print_memory(devtree_memory_t const *node, size_t indent, char const *colour){
	printf("%*s" ATTR("%s", "%s") " (base=%#x, end=%#x, size=%.2fk)\n", indent, "", colour, node->name, node->base, node->base + node->size, (float)node->size / 1024);

	if(node->childs == NULL)
		return;

	for(size_t i=0; node->childs[i]!=NULL; i++)
		print_memory(node->childs[i], indent + 2, FG_KOBALT);
}

static void print_device(devtree_device_t const *node, size_t indent, char const *colour){
	printf("%*s" ATTR("%s", "%s"), indent, "", colour, node->name);

	if(node->compatible != NULL && node->compatible[0] != 0)
		printf(" (driver=%s)", node->compatible);

	printf("\n");

	if(node->childs == NULL)
		return;

	for(size_t i=0; node->childs[i]!=NULL; i++)
		print_device(node->childs[i], indent + 2, FG_KOBALT);
}
