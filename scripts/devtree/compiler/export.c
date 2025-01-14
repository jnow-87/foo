/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/limits.h>
#include <sys/string.h>
#include <sys/vector.h>
#include <sys/list.h>
#include <sys/escape.h>
#include <asserts.h>
#include <nodes.h>
#include <options.h>
#include <parser.tab.h>


/* macros */
#define MAKE_FILE_HEADER(fp)			file_header(fp, "#", "")
#define MAKE_SECTION_HEADER(fp, s, ...)	section_header(fp, "#", "", s, ##__VA_ARGS__)
#define SRC_FILE_HEADER(fp)				file_header(fp, "/*", "*/")
#define SRC_SECTION_HEADER(fp, s, ...)	section_header(fp, "/*", "*/", s, ##__VA_ARGS__)

#define WARN(node, fmt, ...) \
	fprintf(stderr, FG("warning", YELLOW) ":%s: " fmt, node->name, ##__VA_ARGS__)


/* types */
typedef enum{
	GUARD_TOP = 1,
	GUARD_BOTTOM,
} include_guard_location_t;

typedef void (*write_attr_t)(FILE *fp, node_t *node, char const *node_ident);


/* local/static prototypes */
static void traverse(FILE *fp, node_t *node, write_attr_t write_attr);

static void makevars(FILE *fp, node_t *node, char const *node_ident);
static void macros(FILE *fp, node_t *node, char const *node_ident);
static void declaration(FILE *fp, node_t *node, char const *node_ident);
static void definition(FILE *fp, node_t *node, char const *node_ident);

static void def_asserts(FILE *fp, node_t *node, char const *node_ident);
static void def_childs(FILE *fp, node_t *node, char const *node_ident);
static void def_payload(FILE *fp, node_t *node, char const *node_ident);
static void def_attributes(FILE *fp, node_t *node, char const *node_ident);

static void file_header(FILE *fp, char const *start_comment, char const *end_comment);
static void section_header(FILE *fp, char const *start_comment, char const *end_comment, char const *s, ...);
static void src_node_header(FILE *fp, char const *node_ident);
static void include_guard(FILE *fp, include_guard_location_t loc);
static void includes(FILE *fp);

static int def_attr_ignore(node_t *node, attr_t *attr);


/* global functions */
void export_make(FILE *fp, vector_t *nodes){
	node_t **node;


	MAKE_FILE_HEADER(fp);

	vector_for_each(nodes, node){
		MAKE_SECTION_HEADER(fp, "%s variables", (*node)->name);
		traverse(fp, *node, makevars);
	}
}

void export_header(FILE *fp, vector_t *nodes){
	node_t **node;


	SRC_FILE_HEADER(fp);
	include_guard(fp, GUARD_TOP);

	vector_for_each(nodes, node){
		SRC_SECTION_HEADER(fp, "%s macros", (*node)->name);
		traverse(fp, *node, macros);
	}

	include_guard(fp, GUARD_BOTTOM);
}

void export_source(FILE *fp, vector_t *nodes){
	node_t **node;


	SRC_FILE_HEADER(fp);
	includes(fp);

	vector_for_each(nodes, node){
		SRC_SECTION_HEADER(fp, "%s declarations", (*node)->name);
		traverse(fp, *node, declaration);

		SRC_SECTION_HEADER(fp, "%s definitions", (*node)->name);
		traverse(fp, *node, definition);
	}
}


/* local functions */
static void traverse(FILE *fp, node_t *node, write_attr_t write_attr){
	size_t name_len = strlen(node->name);
	char node_ident[name_len + 1];
	node_t *child;


	strcident_r(node->name, node_ident, name_len);
	write_attr(fp, node, node_ident);

	list_for_each(node->childs, child)
		traverse(fp, child, write_attr);
}

static void makevars(FILE *fp, node_t *node, char const *node_ident){
	char node_name[DEVTREE_STRMAX],
		 attr_name[DEVTREE_STRMAX];
	attr_t *attr;
	attr_value_t *v;


#ifdef CONFIG_X86
	// NOTE Do not export the heap memory node for x86 to avoid confusion, since its heap is allocated
	// 		dynamically, cf. the note in macros().
	if(strcmp(node->name, "heap") == 0)
		return;
#endif // CONFIG_X86

	strupr(node_ident, node_name, sizeof(node_name));

	vector_for_each(&node->attrs, attr){
		strupr(attr->name, attr_name, sizeof(attr_name));
		v = &attr->value;

		if(attr->flags & AF_ARRAY)
			continue;

		switch(attr->type){
		case AT_INT8:	// fall through
		case AT_INT16:	// fall through
		case AT_INT32:	// fall through
		case AT_INT64:
			fprintf(fp, "DEVTREE_%s_%s := %#u\n", node_name, attr_name, v->i);
			break;

		case AT_ADDR:	fprintf(fp, "DEVTREE_%s_%s := %#x\n", node_name, attr_name, v->i); break;
		case AT_STRING:	fprintf(fp, "DEVTREE_%s_%s := %#s\n", node_name, attr_name, v->p); break;

		default:
			break;
		}
	}
}

static void macros(FILE *fp, node_t *node, char const *node_ident){
	char node_name[DEVTREE_STRMAX],
		 attr_name[DEVTREE_STRMAX];
	attr_t *attr;
	attr_value_t *v;


	strupr(node_ident, node_name, sizeof(node_name));

	vector_for_each(&node->attrs, attr){
#ifdef CONFIG_X86
		// NOTE On x86 the kernel heap is allocated dynamically and the devtree script only contains
		// 		an artificial base address. However, since DEVTREE_HEAP_BASE is assumed to be valid
		// 		by macros such as KERNEL_STACK(), instead of using the constant from the devtree, the
		// 		macro redirects to the actual devicetree node.
		if(strcmp(node->name, "heap") == 0 && strcmp(attr->name, "base") == 0){
			fprintf(fp, "#define DEVTREE_HEAP_BASE (devtree_find_memory_by_name(&__dt_memory_root, \"heap\")->base)\n");
			continue;
		}
#endif // CONFIG_X86

		strupr(attr->name, attr_name, sizeof(attr_name));
		v = &attr->value;

		if(attr->flags & AF_ARRAY)
			continue;

		switch(attr->type){
		case AT_INT8:	// fall through
		case AT_INT16:	// fall through
		case AT_INT32:	// fall through
		case AT_INT64:
			fprintf(fp, "#define DEVTREE_%s_%s %#u\n", node_name, attr_name, v->i);
			break;

		case AT_ADDR:	fprintf(fp, "#define DEVTREE_%s_%s %#x\n", node_name, attr_name, v->i); break;
		case AT_STRING:	fprintf(fp, "#define DEVTREE_%s_%s %#s\n", node_name, attr_name, v->p); break;

		default:
			break;
		}
	}
}

static void declaration(FILE *fp, node_t *node, char const *node_ident){
	fprintf(fp, "devtree_%s_t const __dt_%s;\n", type_strcat(node->type->category), node_ident);
}

static void definition(FILE *fp, node_t *node, char const *node_ident){
	src_node_header(fp, node_ident);

	def_childs(fp, node, node_ident);

	if(node->type->category == TC_DEVICE)
		def_payload(fp, node, node_ident);

	fprintf(fp, "devtree_%s_t const __dt_%s = {\n", type_strcat(node->type->category), node_ident);

	switch(node->type->category){
	case TC_DEVICE:
		fprintf(fp, "\t.name = \"%s\",\n", node->name);
		fprintf(fp, "\t.compatible = \"%s\",\n", attr_get(&node->attrs, "compatible", false)->value.p);

		if(node->attrs.size > 1)
			fprintf(fp, "\t.payload = &__dt_%s_payload,\n", node_ident);
		else
			fprintf(fp, "\t.payload = 0x0,\n");
		break;

	case TC_MEMORY:
		fprintf(fp, "\t.name = \"%s\",\n", node->name);
		def_attributes(fp, node, node_ident);
		break;

	default:
		WARN(node, "unexpected node type (%d)\n", node->type);
		break;
	}

	if(!list_empty(node->childs))	fprintf(fp, "\t.childs = __dt_%s_childs,\n", node_ident);
	else							fprintf(fp, "\t.childs = 0x0,\n");

	fprintf(fp, "};\n\n\n");

	def_asserts(fp, node, node_ident);
}

static void def_asserts(FILE *fp, node_t *node, char const *node_ident){
	assert_t *assert;


	list_for_each(node->type->asserts, assert){
		fprintf(fp, "_Static_assert(");
		assert_export(fp, assert, &node->attrs);
		fprintf(fp, ", \"%s: %s\");\n", node_ident, assert->msg);
	}

	if(node->type->asserts)
		fprintf(fp, "\n");
}

static void def_childs(FILE *fp, node_t *node, char const *node_ident){
	node_t *child;


	if(list_empty(node->childs))
		return;

	fprintf(fp, "devtree_%s_t const * const __dt_%s_childs[] = {\n"
		, type_strcat(list_first(node->childs)->type->category)
		, node_ident
	);

	list_for_each(node->childs, child)
		fprintf(fp, "\t&__dt_%s,\n", strcident(child->name));

	fprintf(fp, "\t0x0\n};\n\n");
}

static void def_payload(FILE *fp, node_t *node, char const *node_ident){
	attr_t *attr;
	attr_value_t *v;


	// struct definition
	fprintf(fp, "struct{\n");

	vector_for_each(&node->attrs, attr){
		if(def_attr_ignore(node, attr))
			continue;

		v = &attr->value;

		switch(attr->type){
		case AT_INT8:	// fall through
		case AT_INT16:	// fall through
		case AT_INT32:	// fall through
		case AT_INT64:
			fprintf(fp, "\tuint%u_t %s", attr_type_size(attr->type) * 8, attr->name);
			break;

		case AT_ADDR:	fprintf(fp, "\tvoid *%s", attr->name); break;
		case AT_STRING:	fprintf(fp, "\tchar *%s", attr->name); break;
		default:		WARN(node, "unexpected attribute type \n", attr_type_name(attr->type)); break;
		}

		if(attr->flags & AF_ARRAY)
			fprintf(fp, "[%zu]", v->arr.items.size);

		fprintf(fp, ";\n");
	}

	fprintf(fp, "}\n");

	// data
	fprintf(fp, " const __dt_%s_payload = {\n", node_ident);
	def_attributes(fp, node, node_ident);
	fprintf(fp, "};\n\n");
}

static void def_attributes(FILE *fp, node_t *node, char const *node_ident){
	attr_t *attr;
	attr_value_t *v;
	vector_t *items;


	vector_for_each(&node->attrs, attr){
		if(def_attr_ignore(node, attr))
			continue;

		v = &attr->value;

		if(attr->flags & AF_ARRAY){
			items = &v->arr.items;
			fprintf(fp, "\t.%s = {\n", attr->name);

			vector_for_each(items, v){
				switch(attr->type){
				case AT_INT8:	// fall through
				case AT_INT16:	// fall through
				case AT_INT32:	// fall through
				case AT_INT64:
					fprintf(fp, "\t\t%u,\n", v->i);
					break;

				case AT_ADDR:	fprintf(fp, "\t\t(void*)%#x,\n", v->p); break;
				case AT_STRING:	fprintf(fp, "\t\t\"%s\",\n", v->p); break;

				default:
					break;
				}
			}

			fprintf(fp, "\t},\n");
		}
		else{
			switch(attr->type){
			case AT_INT8:	// fall through
			case AT_INT16:	// fall through
			case AT_INT32:	// fall through
			case AT_INT64:
				fprintf(fp, "\t.%s = %u,\n", attr->name, v->i);
				break;

			case AT_ADDR:	fprintf(fp, "\t.%s = (void*)%#x,\n", attr->name, v->i); break;
			case AT_STRING:	fprintf(fp, "\t.%s = \"%s\",\n", attr->name, v->p); break;

			default:
				break;
			}
		}

	}
}

static void file_header(FILE *fp, char const *start_comment, char const *end_comment){
	fprintf(fp,
		"%s generated based on the device tree script \"%s\" %s\n"
		"\n"
		, start_comment
		, options.ifile_name
		, end_comment
	);
}

static void section_header(FILE *fp, char const *start_comment, char const *end_comment, char const *s, ...){
	va_list lst;


	fprintf(fp, "\n\n%s ", start_comment);

	va_start(lst, s);
	vfprintf(fp, s, lst);
	va_end(lst);

	fprintf(fp, " %s\n", end_comment);
}

static void src_node_header(FILE *fp, char const *node_ident){
	fprintf(fp, "// __dt_%s\n", node_ident);
}

static void include_guard(FILE *fp, include_guard_location_t loc){
	size_t len = options.ofile_name ? strlen(options.ofile_name) : 7;
	char guard[len + 1];


	if(options.ofile_name){
		strupr(options.ofile_name, guard, sizeof(guard));

		for(size_t i=0; i<len; i++){
			if(!isalnum(guard[i]))
				guard[i] = '_';
		}
	}
	else
		strcpy(guard, "GUARD_H");

	if(loc == GUARD_TOP){
		fprintf(fp,
			"\n"
			"\n"
			"#ifndef %s\n"
			"#define %s\n"
			, guard
			, guard
		);
	}
	else{
		fprintf(fp,
			"\n"
			"\n"
			"#endif // %s\n"
			, guard
		);
	}
}

static void includes(FILE *fp){
	fprintf(fp,
		"\n"
		"\n"
		"#ifdef BUILD_HOST\n"
		"#include <stdint.h>\n"
		"#else\n"
		"#include <sys/types.h>\n"
		"#endif // BUILD_HOST\n"
		"\n"
		"#include <sys/devtree.h>\n"
	);
}

static int def_attr_ignore(node_t *node, attr_t *attr){
	if(node->type->category == TC_DEVICE){
		if(strcmp(attr->name, "compatible") == 0)
			return 1;
	}

	return 0;
}
