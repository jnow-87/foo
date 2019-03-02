/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/vector.h>
#include <sys/list.h>
#include <node.h>
#include <stdio.h>


/* global functions */
char const *node_validate(node_t *node){
	int have_reg_base,
		have_reg,
		have_int;
	unsigned int x;
	char const *c;
	member_t *m;
	node_t *child;


	/* init */
	have_reg_base = 0;
	have_reg = 0;
	have_int = 0;

	/* check compatible string */
	if(node->compatible == 0x0 || *node->compatible == 0)
		return "empty compatible string";

	/* check attributes */
	vector_for_each(&node->data, m){
		if(m->type == MT_INT_LIST){
			have_int++;

			for(x=8; x<1024; x*=2){
				if(x == ((member_int_t*)m->data)->size)
					break;
			}

			if(x == 1024)
				return "node contains integer type of invalid size";
		}
		else if(m->type == MT_REG_BASE)
			have_reg_base++;
		else if(m->type == MT_REG_LIST)
			have_reg++;
	}

	// only have a single register base address
	if(have_reg_base > 1)
		return "more than one register base address defined";

	// only have either a register base address or register and integer lists
	if(have_reg_base && (have_reg || have_int))
		return "register base address as well as register or integer members defined";

	/* check childs */
	list_for_each(node->childs, child){
		if((c = node_validate(child)) != 0)
			return c;
	}

	return 0x0;
}

void node_export(node_t *node){
	/* TODO */
}

void node_print(node_t *node, size_t indent){
	size_t i;
	unsigned int *p;
	member_int_t *lst;
	member_t *m;
	char s_ind[indent + 1];
	node_t *child;


	for(i=0; i<indent; i++)
		s_ind[i] = '\t';
	s_ind[i] = 0;


	printf("\n%snode: %s\n", s_ind, node->name);
	printf("%s\tcompatible: %s\n", s_ind, node->compatible);
	printf("%s\tparent: %s\n", s_ind, (node->parent != 0x0 ? node->parent->name : ""));

	printf("\n%s\t%u members\n", s_ind, node->data.size);

	vector_for_each(&node->data, m){
		switch(m->type){
		case MT_REG_BASE:
			printf("%s\t\treg-base: %p\n", s_ind, m->data);
			break;

		case MT_REG_LIST:
			printf("%s\t\treg-list (%u):", s_ind, ((vector_t*)m->data)->size);

			vector_for_each((vector_t*)m->data, p)
				printf(" %#x", *p);

			printf("\n");
			break;

		case MT_INT_LIST:
			lst = m->data;
			printf("%s\t\tint<%u>-list (%u):", s_ind, lst->size, lst->lst->size);

			vector_for_each(lst->lst, p)
				printf(" %#x", *p);

			printf("\n");
			break;

		default:
			printf("%s\t\tunknown member type %u\n", s_ind, m->type);
		}
	}

	list_for_each(node->childs, child)
		node_print(child, indent + 1);
}
