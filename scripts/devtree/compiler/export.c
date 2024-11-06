/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <stdio.h>
#include <sys/limits.h>
#include <sys/string.h>
#include <sys/vector.h>
#include <sys/list.h>
#include <sys/escape.h>
#include <asserts.h>
#include <nodes.h>
#include <options.h>


/* macros */
#define INCLUDE_GUARD	"GENERATED_DEVICETREE_H"

#define MAKE_FILE_HEADER(fp)		file_header(fp, "#", "")
#define MAKE_SECTION_HEADER(fp, s)	section_header(fp, "#", "", s)
#define SRC_FILE_HEADER(fp)			file_header(fp, "/*", "*/")
#define SRC_SECTION_HEADER(fp, s)	section_header(fp, "/*", "*/", s)

#define WARN(node, fmt, ...) \
	fprintf(stderr, FG("warning", YELLOW) ":%s: " fmt, node->name, ##__VA_ARGS__)


/* types */
typedef void (*write_attr_t)(FILE *fp, node_t *node, char const *node_ident);


/* local/static prototypes */
static void traverse(FILE *fp, node_t *node, write_attr_t write_attr);

static void device_makevars(FILE *fp, node_t *node, char const *node_ident);
static void memory_makevars(FILE *fp, node_t *node, char const *node_ident);
static void arch_makevars(FILE *fp, node_t *node, char const *node_ident);
static void device_macros(FILE *fp, node_t *node, char const *node_ident);
static void memory_macros(FILE *fp, node_t *node, char const *node_ident);
static void arch_macros(FILE *fp, node_t *node, char const *node_ident);
static void base_declaration(FILE *fp, node_t *node, char const *node_ident);
static void base_definition(FILE *fp, node_t *node, char const *node_ident);
static void device_definition(FILE *fp, node_t *node, char const *node_ident);
static void memory_definition(FILE *fp, node_t *node, char const *node_ident);
static void arch_definition(FILE *fp, node_t *node, char const *node_ident);
static void base_asserts(FILE *fp, node_t *node, char const *node_ident);

static void file_header(FILE *fp, char const *start_comment, char const *end_comment);
static void section_header(FILE *fp, char const *start_comment, char const *end_comment, char const *s);
static void src_node_header(FILE *fp, char const *node_ident);
static void include_guard_top(FILE *fp);
static void include_guard_bottom(FILE *fp);
static void includes(FILE *fp);
static void childs(FILE *fp, node_t *node, char const *node_ident);
static void attributes(FILE *fp, node_t *node, char const *node_ident);


/* static variables */
static char const *node_type_names[] = {
	"",
	"device",	// NT_DEVICE
	"memory",	// NT_MEMORY
	"arch",		// NT_ARCH
};


/* global functions */
void export_make(FILE *fp){
	MAKE_FILE_HEADER(fp);

	if(options.export_sections & DT_DEVICES){
		MAKE_SECTION_HEADER(fp, "device variables");
		traverse(fp, device_root(), device_makevars);
	}

	if(options.export_sections & DT_MEMORY){
		MAKE_SECTION_HEADER(fp, "memory variables");
		traverse(fp, memory_root(), memory_makevars);
	}

	if(options.export_sections & DT_ARCH){
		MAKE_SECTION_HEADER(fp, "arch variables");
		traverse(fp, arch_root(), arch_makevars);
	}
}

void export_header(FILE *fp){
	SRC_FILE_HEADER(fp);
	include_guard_top(fp);

	if(options.export_sections & DT_DEVICES){
		SRC_SECTION_HEADER(fp, "device macros");
		traverse(fp, device_root(), device_macros);
	}

	if(options.export_sections & DT_MEMORY){
		SRC_SECTION_HEADER(fp, "memory macros");
		traverse(fp, memory_root(), memory_macros);
	}

	if(options.export_sections & DT_ARCH){
		SRC_SECTION_HEADER(fp, "arch macros");
		traverse(fp, arch_root(), arch_macros);
	}

	include_guard_bottom(fp);
}

void export_source(FILE *fp){
	SRC_FILE_HEADER(fp);
	includes(fp);

	if(options.export_sections & DT_DEVICES){
		SRC_SECTION_HEADER(fp, "device node declarations");
		traverse(fp, device_root(), base_declaration);

		SRC_SECTION_HEADER(fp, "device node definitions");
		traverse(fp, device_root(), base_definition);
	}

	if(options.export_sections & DT_MEMORY){
		SRC_SECTION_HEADER(fp, "memory node declarations");
		traverse(fp, memory_root(), base_declaration);

		SRC_SECTION_HEADER(fp, "memory node definitions");
		traverse(fp, memory_root(), base_definition);
	}

	if(options.export_sections & DT_ARCH){
		SRC_SECTION_HEADER(fp, "arch node declarations");
		traverse(fp, arch_root(), base_declaration);

		SRC_SECTION_HEADER(fp, "arch node definitions");
		traverse(fp, arch_root(), base_definition);
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

static void device_makevars(FILE *fp, node_t *node, char const *node_ident){
	fprintf(fp, "DEVTREE_%s_COMPATIBLE := %zu\n", strupr(node_ident), node_attr_get(node, MT_COMPATIBLE, 0)->p);
}

static void memory_makevars(FILE *fp, node_t *node, char const *node_ident){
	// NOTE Do not export memory nodes for x86 to avoid confusion, since its heap is allocated
	// 		dynamically, cf. the note in memory_macros().
#ifndef CONFIG_X86
	node_ident = strupr(node_ident);

	fprintf(fp, "DEVTREE_%s_BASE := %#lx\n", node_ident, node_attr_get(node, MT_BASE_ADDR, 0)->i);
	fprintf(fp, "DEVTREE_%s_SIZE := %zu\n", node_ident, node_attr_get(node, MT_SIZE, 0)->i);
#endif // CONFIG_X86
}

static void arch_makevars(FILE *fp, node_t *node, char const *node_ident){
	bool multi_core;


	if(node->type != NT_ARCH)
		return;

	multi_core = (node_attr_get(node, MT_NCORES, 0)->i > 1);

	fprintf(fp, "DEVTREE_ARCH_ADDR_WIDTH := %u\n", node_attr_get(node, MT_ADDR_WIDTH, 0)->i);
	fprintf(fp, "DEVTREE_ARCH_REG_WIDTH := %u\n", node_attr_get(node, MT_REG_WIDTH, 0)->i);
	fprintf(fp, "DEVTREE_ARCH_CORE_MASK := %#x\n", node_attr_get(node, MT_CORE_MASK, 0)->i);
	fprintf(fp, "DEVTREE_ARCH_NCORES := %u\n", node_attr_get(node, MT_NCORES, 0)->i);

	if(multi_core)
		fprintf(fp, "DEVTREE_ARCH_MULTI_CORE := y\n");

	fprintf(fp, "DEVTREE_ARCH_NUM_INTS := %u\n", node_attr_get(node, MT_NUM_INTS, 0)->i);
	fprintf(fp, "DEVTREE_ARCH_TIMER_INT := %u\n", node_attr_get(node, MT_TIMER_INT, 0)->i);
	fprintf(fp, "DEVTREE_ARCH_SYSCALL_INT := %u\n", node_attr_get(node, MT_SYSCALL_INT, 0)->i);

	if(multi_core)
		fprintf(fp, "DEVTREE_ARCH_IPI_INT := %u\n", node_attr_get(node, MT_IPI_INT, 0)->i);

	fprintf(fp, "DEVTREE_ARCH_TIMER_CYCLE_TIME_US := %u\n", node_attr_get(node, MT_TIMER_CYCLE_TIME_US, 0)->i);
}

static void device_macros(FILE *fp, node_t *node, char const *node_ident){
	fprintf(fp, "#define DEVTREE_%s_COMPATIBLE %zu\n", strupr(node_ident), node_attr_get(node, MT_COMPATIBLE, 0)->p);
}

static void memory_macros(FILE *fp, node_t *node, char const *node_ident){
	node_ident = strupr(node_ident);

#ifdef CONFIG_X86
	// NOTE On x86 the kernel heap is allocated dynamically and the devtree script only contains
	// 		an artificial base address. However, since DEVTREE_HEAP_BASE is assumed to be valid
	// 		by macros such as KERNEL_STACK(), instead of using the constant from the devtree, the
	// 		macro redirects to the actual devicetree node.
	if(strcmp(node_ident, "HEAP") == 0){
		fprintf(fp, "#define DEVTREE_HEAP_BASE (devtree_find_memory_by_name(&__dt_memory_root, \"heap\")->base)\n");
	}
	else
#endif // CONFIG_X86
		fprintf(fp, "#define DEVTREE_%s_BASE %#lx\n", node_ident, node_attr_get(node, MT_BASE_ADDR, 0)->i);

	fprintf(fp, "#define DEVTREE_%s_SIZE %zu\n", node_ident, node_attr_get(node, MT_SIZE, 0)->i);
}

static void arch_macros(FILE *fp, node_t *node, char const *node_ident){
	bool multi_core;


	if(node->type != NT_ARCH)
		return;

	multi_core = (node_attr_get(node, MT_NCORES, 0)->i > 1);

	fprintf(fp, "#define DEVTREE_ARCH_ADDR_WIDTH %u\n", node_attr_get(node, MT_ADDR_WIDTH, 0)->i);
	fprintf(fp, "#define DEVTREE_ARCH_REG_WIDTH %u\n", node_attr_get(node, MT_REG_WIDTH, 0)->i);
	fprintf(fp, "#define DEVTREE_ARCH_CORE_MASK %#x\n", node_attr_get(node, MT_CORE_MASK, 0)->i);
	fprintf(fp, "#define DEVTREE_ARCH_NCORES %u\n", node_attr_get(node, MT_NCORES, 0)->i);

	if(multi_core)
		fprintf(fp, "#define DEVTREE_ARCH_MULTI_CORE\n");

	fprintf(fp, "#define DEVTREE_ARCH_NUM_INTS %u\n", node_attr_get(node, MT_NUM_INTS, 0)->i);
	fprintf(fp, "#define DEVTREE_ARCH_TIMER_INT %u\n", node_attr_get(node, MT_TIMER_INT, 0)->i);
	fprintf(fp, "#define DEVTREE_ARCH_SYSCALL_INT %u\n", node_attr_get(node, MT_SYSCALL_INT, 0)->i);

	if(multi_core)
		fprintf(fp, "#define DEVTREE_ARCH_IPI_INT %u\n", node_attr_get(node, MT_IPI_INT, 0)->i);

	fprintf(fp, "#define DEVTREE_ARCH_TIMER_CYCLE_TIME_US %u\n", node_attr_get(node, MT_TIMER_CYCLE_TIME_US, 0)->i);
}

static void base_declaration(FILE *fp, node_t *node, char const *node_ident){
	fprintf(fp, "devtree_%s_t const __dt_%s;\n", node_type_names[node->type], node_ident);
}

static void base_definition(FILE *fp, node_t *node, char const *node_ident){
	src_node_header(fp, node_ident);

	base_asserts(fp, node, node_ident);
	childs(fp, node, node_ident);

	if(node->type == NT_DEVICE)
		attributes(fp, node, node_ident);

	fprintf(fp, "devtree_%s_t const __dt_%s = {\n", node_type_names[node->type], node_ident);

	switch(node->type){
	case NT_DEVICE:	device_definition(fp, node, node_ident); break;
	case NT_MEMORY:	memory_definition(fp, node, node_ident); break;
	case NT_ARCH:	arch_definition(fp, node, node_ident); break;

	default:
		WARN(node, "unexpected node type (%d)\n", node->type);
		break;
	}

	if(!list_empty(node->childs))	fprintf(fp, "\t.childs = __dt_%s_childs,\n", node_ident);
	else							fprintf(fp, "\t.childs = 0x0,\n");

	fprintf(fp, "};\n\n\n");
}

static void device_definition(FILE *fp, node_t *node, char const *node_ident){
	attr_t *a;


	fprintf(fp, "\t.name = \"%s\",\n", node->name);
	fprintf(fp, "\t.compatible = \"%s\",\n", node_attr_get(node, MT_COMPATIBLE, 0)->p);

	if(node->attrs.size > 1){
		a = vector_get(&node->attrs, 0);

		if(a->value.p != 0x0){
			fprintf(fp, "\t.payload = &__dt_%s_payload,\n", node_ident);
			return;
		}
	}

	fprintf(fp, "\t.payload = 0x0,\n");
}

static void memory_definition(FILE *fp, node_t *node, char const *node_ident){
	fprintf(fp, "\t.name = \"%s\",\n", node->name);
	fprintf(fp, "\t.base = (void*)%#x,\n", node_attr_get(node, MT_BASE_ADDR, 0)->i);
	fprintf(fp, "\t.size = %zu,\n", node_attr_get(node, MT_SIZE, 0)->i);
}

static void arch_definition(FILE *fp, node_t *node, char const *node_ident){
	bool multi_core;


	if(node->type == NT_ARCH){
		multi_core = (node_attr_get(node, MT_NCORES, 0)->i > 1);

		fprintf(fp, "\t.addr_width = %u,\n", node_attr_get(node, MT_ADDR_WIDTH, 0)->i);
		fprintf(fp, "\t.reg_width = %u,\n", node_attr_get(node, MT_REG_WIDTH, 0)->i);
		fprintf(fp, "\t.core_mask = %#x,\n", node_attr_get(node, MT_CORE_MASK, 0)->i);
		fprintf(fp, "\t.ncores = %u,\n", node_attr_get(node, MT_NCORES, 0)->i);
		fprintf(fp, "\t.num_ints = %u,\n", node_attr_get(node, MT_NUM_INTS, 0)->i);
		fprintf(fp, "\t.timer_int = %u,\n", node_attr_get(node, MT_TIMER_INT, 0)->i);
		fprintf(fp, "\t.syscall_int = %u,\n", node_attr_get(node, MT_SYSCALL_INT, 0)->i);

		if(multi_core)
			fprintf(fp, "\t.ipi_int = %u,\n", node_attr_get(node, MT_IPI_INT, 0)->i);

		fprintf(fp, "\t.timer_cycle_time_us = %u,\n", node_attr_get(node, MT_TIMER_CYCLE_TIME_US, 0)->i);
	}
	else if(node->type == NT_DEVICE)
		device_definition(fp, node, node_ident);
}

static void base_asserts(FILE *fp, node_t *node, char const *node_ident){
	assert_t *a;


	list_for_each(node->asserts, a){
		fprintf(fp, "_Static_assert(%s, \"%s: %s\");\n", a->expr, node_ident, a->msg);
	}

	if(node->asserts)
		fprintf(fp, "\n");
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

static void section_header(FILE *fp, char const *start_comment, char const *end_comment, char const *s){
	fprintf(fp, "\n\n%s %s %s\n", start_comment, s, end_comment);
}

static void src_node_header(FILE *fp, char const *node_ident){
	fprintf(fp, "// __dt_%s\n", node_ident);
}

static void include_guard_top(FILE *fp){
	fprintf(fp,
		"\n"
		"\n"
		"#ifndef " INCLUDE_GUARD "\n"
		"#define " INCLUDE_GUARD "\n"
	);
}

static void include_guard_bottom(FILE *fp){
	fprintf(fp,
		"\n"
		"\n"
		"#endif // " INCLUDE_GUARD "\n"
	);
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

static void childs(FILE *fp, node_t *node, char const *node_ident){
	node_t *child;


	if(list_empty(node->childs))
		return;

	fprintf(fp, "devtree_%s_t const * const __dt_%s_childs[] = {\n"
		, node_type_names[list_first(node->childs)->type]
		, node_ident
	);

	list_for_each(node->childs, child)
		fprintf(fp, "\t&__dt_%s,\n", strcident(child->name));

	fprintf(fp, "\t0x0\n};\n\n");
}

static void attributes(FILE *fp, node_t *node, char const *node_ident){
	size_t n_int = 0,
		   n_reg = 0,
		   n_base = 0;
	unsigned long int *p;
	attr_t *a;


	if(node->attrs.size < 2)
		return;

	// struct definition
	fprintf(fp, "struct{\n");

	vector_for_each(&node->attrs, a){
		switch(a->type){
		case MT_BASE_ADDR:	fprintf(fp, "\tvoid *base%zu;\n", n_base++); break;
		case MT_STRING:		fprintf(fp, "\tchar *string%zu;\n", n_base++); break;

		case MT_REG_LIST:
			vector_for_each(a->value.lst->items, p)
				fprintf(fp, "\tvoid *reg%zu;\n", n_reg++);
			break;

		case MT_INT_LIST:
			vector_for_each(a->value.lst->items, p)
				fprintf(fp, "\tuint%d_t int%zu;\n", a->value.lst->type_size, n_int++);
			break;

		case MT_COMPATIBLE:
			break;

		default:
			WARN(node, "unexpected attribute type (%d)\n", a->type);
			break;
		}
	}

	fprintf(fp, "}\n");

	// data
	fprintf(fp, " const __dt_%s_payload = {\n", node_ident);

	n_int = 0;
	n_reg = 0;
	n_base = 0;

	vector_for_each(&node->attrs, a){
		switch(a->type){
		case MT_BASE_ADDR:	fprintf(fp, "\t.base%zu = (void*)%#x,\n", n_base++, a->value.i); break;
		case MT_STRING:		fprintf(fp, "\t.string%zu = \"%s\",\n", n_base++, a->value.p); break;

		case MT_REG_LIST:
			vector_for_each(a->value.lst->items, p)
				fprintf(fp, "\t.reg%zu = (void*)%#x,\n", n_reg++, *p);
			break;

		case MT_INT_LIST:
			vector_for_each(a->value.lst->items, p)
				fprintf(fp, "\t.int%zu = %u,\n", n_int++, *p);
			break;

		default:
			// already reported above
			break;
		}
	}

	fprintf(fp, "};\n\n");
}
