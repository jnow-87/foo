/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/vector.h>
#include <sys/list.h>
#include <sys/escape.h>
#include <node.h>
#include <stdio.h>


/* global functions */
int node_export_devices(device_node_t *node, FILE *fp){
	int r;
	size_t n_int,
		   n_reg,
		   n_base;
	unsigned int *p;
	member_int_t *int_lst;
	member_t *m;
	device_node_t *child;


	fprintf(fp, "/**\n *\t__dt_%s\n */\n", node->name);

	/* child list */
	if(!list_empty(node->childs)){
		// child declarations
		fprintf(fp, "// __dt_%s child declarations\n", node->name);

		list_for_each(node->childs, child)
			fprintf(fp, "devtree_device_t const __dt_%s;\n", child->name);

		fprintf(fp, "\n");

		// child array
		fprintf(fp, "// __dt_%s child list\n", node->name);

		fprintf(fp, "devtree_device_t const * const __dt_%s_childs[] = {\n", node->name);

		list_for_each(node->childs, child)
			fprintf(fp, "\t&__dt_%s,\n", child->name);

		fprintf(fp, "\t0x0\n};\n\n");
	}

	/* data */
	m = vector_get(&node->data, 0);

	if(node->data.size > 1 || (m != 0x0 && m->type != MT_BASE_ADDR)){
		n_int = 0;
		n_reg = 0;
		n_base = 0;

		fprintf(fp, "// __dt_%s data\n", node->name);

		// struct definition
		fprintf(fp, "struct{\n");

		vector_for_each(&node->data, m){
			switch(m->type){
			case MT_REG_LIST:
				vector_for_each((vector_t*)m->data, p)
					fprintf(fp, "\tvoid *reg%zu;\n", n_reg++);
				break;

			case MT_INT_LIST:
				int_lst = m->data;

				vector_for_each(int_lst->lst, p)
					fprintf(fp, "\tuint%d_t int%zu;\n", int_lst->size, n_int++);
				break;

			case MT_BASE_ADDR:
				fprintf(fp, "\tvoid *base%zu;\n", n_base++);
				break;

			case MT_SIZE:
			default:
				fprintf(stderr, FG_RED "error" RESET_ATTR ": unexpected member type (%d) in node \"%s\"\n", m->type, node->name);
				return -1;
			}
		}

		fprintf(fp, "} __attribute__((packed))\n");

		n_int = 0;
		n_reg = 0;
		n_base = 0;

		// data
		fprintf(fp, " const __dt_%s_data = {\n", node->name);

		vector_for_each(&node->data, m){
			switch(m->type){
			case MT_REG_LIST:
				vector_for_each((vector_t*)m->data, p)
					fprintf(fp, "\t.reg%zu = (void*)%#x,\n", n_reg++, *p);
				break;

			case MT_INT_LIST:
				int_lst = m->data;

				vector_for_each(int_lst->lst, p)
					fprintf(fp, "\t.int%zu = %u,\n", n_int++, *p);
				break;

			case MT_BASE_ADDR:
				fprintf(fp, "\t.base%zu = (void*)%#x,\n", n_base++, m->data);
				break;

			case MT_SIZE:
			default:
				// already exited above
				break;
			}
		}

		fprintf(fp, "};\n\n");
	}

	/* node definition */
	fprintf(fp, "// __dt_%s definition\n", node->name);
	fprintf(fp, "devtree_device_t const __dt_%s = {\n", node->name);

	fprintf(fp, "\t.name = \"%s\",\n", node->name);
	fprintf(fp, "\t.compatible = \"%s\",\n", node->compatible);

	m = vector_get(&node->data, 0);

	if(node->data.size == 1 && m->type == MT_BASE_ADDR)	fprintf(fp, "\t.data = %p,\n", m->data);
	else if(m == 0x0 || m->data == 0x0)					fprintf(fp, "\t.data = 0x0,\n");
	else												fprintf(fp, "\t.data = &__dt_%s_data,\n", node->name);

	if(node->parent)									fprintf(fp, "\t.parent = &__dt_%s,\n", node->parent->name);
	else												fprintf(fp, "\t.parent = 0x0,\n");

	if(!list_empty(node->childs))						fprintf(fp, "\t.childs = __dt_%s_childs,\n", node->name);
	else												fprintf(fp, "\t.childs = 0x0,\n");

	fprintf(fp, "};\n\n\n");

	/* export childs */
	list_for_each(node->childs, child){
		if((r = node_export_devices(child, fp)))
			return r;
	}

	return 0;
}

int node_export_memory(memory_node_t *node, FILE *fp){
	int r;
	memory_node_t *child;


	fprintf(fp, "/**\n *\t__dt_%s\n */\n", node->name);

	/* child list */
	if(!list_empty(node->childs)){
		// child declarations
		fprintf(fp, "// __dt_%s child declarations\n", node->name);

		list_for_each(node->childs, child)
			fprintf(fp, "devtree_memory_t const __dt_%s;\n", child->name);

		fprintf(fp, "\n");

		// child array
		fprintf(fp, "// __dt_%s child list\n", node->name);

		fprintf(fp, "devtree_memory_t const * const __dt_%s_childs[] = {\n", node->name);

		list_for_each(node->childs, child)
			fprintf(fp, "\t&__dt_%s,\n", child->name);

		fprintf(fp, "\t0x0\n};\n\n");
	}

	/* node definition */
	fprintf(fp, "// __dt_%s definition\n", node->name);
	fprintf(fp, "devtree_memory_t const __dt_%s = {\n", node->name);

	fprintf(fp, "\t.name = \"%s\",\n", node->name);
	fprintf(fp, "\t.base = (void*)%#x,\n", node->base);
	fprintf(fp, "\t.size = %zu,\n", node->size);

	if(node->parent)				fprintf(fp, "\t.parent = &__dt_%s,\n", node->parent->name);
	else							fprintf(fp, "\t.parent = 0x0,\n");

	if(!list_empty(node->childs))	fprintf(fp, "\t.childs = __dt_%s_childs,\n", node->name);
	else							fprintf(fp, "\t.childs = 0x0,\n");

	fprintf(fp, "};\n\n\n");

	/* export childs */
	list_for_each(node->childs, child){
		if((r = node_export_memory(child, fp)))
			return r;
	}

	return 0;
}
