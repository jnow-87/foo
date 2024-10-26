/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/compiler.h>
#include <sys/types.h>
#include <sys/vector.h>
#include <stdlib.h>
#include <attr.h>
#include <parser.tab.h>


/* global functions */
char const *attr_name(attr_type_t type){
	static char const *names[] = {
		"unknown",
		"compatible",
		"baseaddr",
		"reg",
		"int-list",
		"string",
		"int",
		"size",
		"addr-width",
		"reg-width",
		"ncores",
		"num-ints",
		"timer-int",
		"syscall-int",
		"ipi-int",
		"timer-cycle-time-us",
	};


	if(type < 0 || type >= sizeof_array(names))
		return names[0];

	return names[type];
}

void *attr_ilist_create(size_t size, vector_t *items){
	attr_list_t *lst;


	lst = malloc(sizeof(attr_list_t));

	if(lst == 0x0)
		goto err;

	lst->type_size = size;
	lst->items = items;

	return lst;


err:
	devtree_parser_error("intlist allocation failed");

	return 0x0;
}

vector_t *ilist_create(void){
	vector_t *v;


	v = malloc(sizeof(vector_t));

	if(v == 0x0)
		goto err_0;

	if(vector_init(v, sizeof(unsigned long int), 16) != 0)
		goto err_1;

	return v;


err_1:
	free(v);

err_0:
	devtree_parser_error("intlist allocation failed");

	return 0x0;
}

int ilist_add(vector_t *lst, unsigned long int v){
	if(vector_add(lst, &v) != 0)
		return devtree_parser_error("intlist extension failed");

	return 0;
}
