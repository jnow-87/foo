/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <sys/limits.h>
#include <sys/string.h>
#include <sys/vector.h>
#include <sys/list.h>
#include <sys/escape.h>
#include <options.h>
#include <nodes.h>


/* macros */
#define INCLUDE_GUARD	"GENERATED_DEVICETREE_H"

#define MAKE_FILE_HEADER(fp)		file_header(fp, "#", "")
#define MAKE_SECTION_HEADER(fp, s)	section_header(fp, "#", "", s)
#define SRC_FILE_HEADER(fp)			file_header(fp, "/*", "*/")
#define SRC_SECTION_HEADER(fp, s)	section_header(fp, "/*", "*/", s)

#define WARN(node, fmt, ...) \
	fprintf(stderr, FG("warning", YELLOW) ":%s: " fmt, node->name, ##__VA_ARGS__)


/* types */
typedef void (*write_attr_t)(FILE *fp, base_node_t *node, char const *node_ident);


/* local/static prototypes */
static void traverse(FILE *fp, base_node_t *node, write_attr_t write_attr);

static void device_makevars(FILE *fp, base_node_t *node, char const *node_ident);
static void memory_makevars(FILE *fp, base_node_t *node, char const *node_ident);
static void arch_makevars(FILE *fp, base_node_t *node, char const *node_ident);
static void device_macros(FILE *fp, base_node_t *node, char const *node_ident);
static void memory_macros(FILE *fp, base_node_t *node, char const *node_ident);
static void arch_macros(FILE *fp, base_node_t *node, char const *node_ident);
static void base_declaration(FILE *fp, base_node_t *node, char const *node_ident);
static void base_definition(FILE *fp, base_node_t *node, char const *node_ident);
static void device_definition(FILE *fp, base_node_t *node, char const *node_ident);
static void memory_definition(FILE *fp, base_node_t *node, char const *node_ident);
static void arch_definition(FILE *fp, base_node_t *node, char const *node_ident);

static void file_header(FILE *fp, char const *start_comment, char const *end_comment);
static void section_header(FILE *fp, char const *start_comment, char const *end_comment, char const *s);
static void src_node_header(FILE *fp, char const *node_ident);
static void include_guard_top(FILE *fp);
static void include_guard_bottom(FILE *fp);
static void includes(FILE *fp);
static void childs(FILE *fp, base_node_t *node, char const *node_ident);
static void payload(FILE *fp, device_node_t *node, char const *node_ident);


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
		traverse(fp, (base_node_t*)device_root(), device_makevars);
	}

	if(options.export_sections & DT_MEMORY){
		MAKE_SECTION_HEADER(fp, "memory variables");
		traverse(fp, (base_node_t*)memory_root(), memory_makevars);
	}

	if(options.export_sections & DT_ARCH){
		MAKE_SECTION_HEADER(fp, "arch variables");
		traverse(fp, (base_node_t*)arch_root(), arch_makevars);
	}
}

void export_header(FILE *fp){
	SRC_FILE_HEADER(fp);
	include_guard_top(fp);

	if(options.export_sections & DT_DEVICES){
		SRC_SECTION_HEADER(fp, "device macros");
		traverse(fp, (base_node_t*)device_root(), device_macros);
	}

	if(options.export_sections & DT_MEMORY){
		SRC_SECTION_HEADER(fp, "memory macros");
		traverse(fp, (base_node_t*)memory_root(), memory_macros);
	}

	if(options.export_sections & DT_ARCH){
		SRC_SECTION_HEADER(fp, "arch macros");
		traverse(fp, (base_node_t*)arch_root(), arch_macros);
	}

	include_guard_bottom(fp);
}

void export_source(FILE *fp){
	SRC_FILE_HEADER(fp);
	includes(fp);

	if(options.export_sections & DT_DEVICES){
		SRC_SECTION_HEADER(fp, "device node declarations");
		traverse(fp, (base_node_t*)device_root(), base_declaration);

		SRC_SECTION_HEADER(fp, "device node definitions");
		traverse(fp, (base_node_t*)device_root(), base_definition);
	}

	if(options.export_sections & DT_MEMORY){
		SRC_SECTION_HEADER(fp, "memory node declarations");
		traverse(fp, (base_node_t*)memory_root(), base_declaration);

		SRC_SECTION_HEADER(fp, "memory node definitions");
		traverse(fp, (base_node_t*)memory_root(), base_definition);
	}

	if(options.export_sections & DT_ARCH){
		SRC_SECTION_HEADER(fp, "arch node declarations");
		traverse(fp, (base_node_t*)arch_root(), base_declaration);

		SRC_SECTION_HEADER(fp, "arch node definitions");
		traverse(fp, (base_node_t*)arch_root(), base_definition);
	}
}


/* local functions */
static void traverse(FILE *fp, base_node_t *node, write_attr_t write_attr){
	size_t name_len = strlen(node->name);
	char node_ident[name_len + 1];
	base_node_t *child;


	strcident_r(node->name, node_ident, name_len);
	write_attr(fp, node, node_ident);

	list_for_each(node->childs, child)
		traverse(fp, child, write_attr);
}

static void device_makevars(FILE *fp, base_node_t *node, char const *node_ident){
	fprintf(fp, "DEVTREE_%s_COMPATIBLE := %zu\n", strupr(node_ident), ((device_node_t*)node)->compatible);
}

static void memory_makevars(FILE *fp, base_node_t *node, char const *node_ident){
	memory_node_t *mem = (memory_node_t*)node;


	node_ident = strupr(node_ident);

	fprintf(fp, "DEVTREE_%s_BASE := %#lx\n", node_ident, mem->base);
	fprintf(fp, "DEVTREE_%s_SIZE := %zu\n", node_ident, mem->size);
}

static void arch_makevars(FILE *fp, base_node_t *node, char const *node_ident){
	arch_node_t *arch = (arch_node_t*)node;


	if(node->type != NT_ARCH)
		return;

	fprintf(fp, "DEVTREE_ARCH_ADDR_WIDTH := %u\n", arch->addr_width);
	fprintf(fp, "DEVTREE_ARCH_REG_WIDTH := %u\n", arch->reg_width);
	fprintf(fp, "DEVTREE_ARCH_CORE_MASK := %#x\n", arch->core_mask);
	fprintf(fp, "DEVTREE_ARCH_NCORES := %u\n", arch->ncores);
	fprintf(fp, "DEVTREE_ARCH_NUM_INTS := %u\n", arch->num_ints);
	fprintf(fp, "DEVTREE_ARCH_NUM_VINTS := %u\n", arch->num_vints);
	fprintf(fp, "DEVTREE_ARCH_TIMER_INT := %u\n", arch->timer_int);
	fprintf(fp, "DEVTREE_ARCH_SYSCALL_INT := %u\n", arch->syscall_int);
	fprintf(fp, "DEVTREE_ARCH_IPI_INT := %u\n", arch->ipi_int);
	fprintf(fp, "DEVTREE_ARCH_TIMER_CYCLE_TIME_US := %u\n", arch->timer_cycle_time_us);
}

static void device_macros(FILE *fp, base_node_t *node, char const *node_ident){
	fprintf(fp, "#define DEVTREE_%s_COMPATIBLE %zu\n", strupr(node_ident), ((device_node_t*)node)->compatible);
}

static void memory_macros(FILE *fp, base_node_t *node, char const *node_ident){
	memory_node_t *mem = (memory_node_t*)node;


	node_ident = strupr(node_ident);

	fprintf(fp, "#define DEVTREE_%s_BASE %#lx\n", node_ident, mem->base);
	fprintf(fp, "#define DEVTREE_%s_SIZE %zu\n", node_ident, mem->size);
}

static void arch_macros(FILE *fp, base_node_t *node, char const *node_ident){
	arch_node_t *arch = (arch_node_t*)node;


	if(arch->type != NT_ARCH)
		return;

	fprintf(fp, "#define DEVTREE_ARCH_ADDR_WIDTH %u\n", arch->addr_width);
	fprintf(fp, "#define DEVTREE_ARCH_REG_WIDTH %u\n", arch->reg_width);
	fprintf(fp, "#define DEVTREE_ARCH_CORE_MASK %#x\n", arch->core_mask);
	fprintf(fp, "#define DEVTREE_ARCH_NCORES %u\n", arch->ncores);
	fprintf(fp, "#define DEVTREE_ARCH_NUM_INTS %u\n", arch->num_ints);
	fprintf(fp, "#define DEVTREE_ARCH_NUM_VINTS %u\n", arch->num_vints);
	fprintf(fp, "#define DEVTREE_ARCH_TIMER_INT %u\n", arch->timer_int);
	fprintf(fp, "#define DEVTREE_ARCH_SYSCALL_INT %u\n", arch->syscall_int);
	fprintf(fp, "#define DEVTREE_ARCH_IPI_INT %u\n", arch->ipi_int);
	fprintf(fp, "#define DEVTREE_ARCH_TIMER_CYCLE_TIME_US %u\n", arch->timer_cycle_time_us);
}

static void base_declaration(FILE *fp, base_node_t *node, char const *node_ident){
	fprintf(fp, "devtree_%s_t const __dt_%s;\n", node_type_names[node->type], node_ident);
}

static void base_definition(FILE *fp, base_node_t *node, char const *node_ident){
	src_node_header(fp, node_ident);

	childs(fp, node, node_ident);

	if(node->type == NT_DEVICE)
		payload(fp, (device_node_t*)node, node_ident);

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

static void device_definition(FILE *fp, base_node_t *node, char const *node_ident){
	device_node_t *dev = (device_node_t*)node;
	member_t *m;


	m = vector_get(&dev->payload, 0);

	fprintf(fp, "\t.name = \"%s\",\n", dev->name);
	fprintf(fp, "\t.compatible = \"%s\",\n", dev->compatible);

	if(m == 0x0 || m->payload == 0x0)	fprintf(fp, "\t.payload = 0x0,\n");
	else								fprintf(fp, "\t.payload = &__dt_%s_payload,\n", node_ident);
}

static void memory_definition(FILE *fp, base_node_t *node, char const *node_ident){
	memory_node_t *mem = (memory_node_t*)node;


	fprintf(fp, "\t.name = \"%s\",\n", mem->name);
	fprintf(fp, "\t.base = (void*)%#x,\n", mem->base);
	fprintf(fp, "\t.size = %zu,\n", mem->size);
}

static void arch_definition(FILE *fp, base_node_t *node, char const *node_ident){
	arch_node_t *arch = (arch_node_t*)node;


	if(node->type == NT_ARCH){
		fprintf(fp, "\t.addr_width = %u,\n", arch->addr_width);
		fprintf(fp, "\t.reg_width = %u,\n", arch->reg_width);
		fprintf(fp, "\t.core_mask = %#x,\n", arch->core_mask);
		fprintf(fp, "\t.ncores = %u,\n", arch->ncores);
		fprintf(fp, "\t.num_ints = %u,\n", arch->num_ints);
		fprintf(fp, "\t.num_vints = %u,\n", arch->num_vints);
		fprintf(fp, "\t.timer_int = %u,\n", arch->timer_int);
		fprintf(fp, "\t.syscall_int = %u,\n", arch->syscall_int);
		fprintf(fp, "\t.ipi_int = %u,\n", arch->ipi_int);
		fprintf(fp, "\t.timer_cycle_time_us = %u,\n", arch->timer_cycle_time_us);
	}
	else if(node->type == NT_DEVICE)
		device_definition(fp, node, node_ident);
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

static void childs(FILE *fp, base_node_t *node, char const *node_ident){
	base_node_t *child;


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

static void payload(FILE *fp, device_node_t *node, char const *node_ident){
	size_t n_int = 0,
		   n_reg = 0,
		   n_base = 0;
	unsigned int *p;
	member_t *m;
	member_int_t *int_lst;


	if(vector_get(&node->payload, 0) == 0x0)
		return;

	// struct definition
	fprintf(fp, "struct{\n");

	vector_for_each(&node->payload, m){
		switch(m->type){
		case MT_BASE_ADDR:	fprintf(fp, "\tvoid *base%zu;\n", n_base++); break;
		case MT_STRING:		fprintf(fp, "\tchar *string%zu;\n", n_base++); break;

		case MT_REG_LIST:
			vector_for_each((vector_t*)m->payload, p)
				fprintf(fp, "\tvoid *reg%zu;\n", n_reg++);
			break;

		case MT_INT_LIST:
			int_lst = m->payload;

			vector_for_each(int_lst->lst, p)
				fprintf(fp, "\tuint%d_t int%zu;\n", int_lst->size, n_int++);
			break;

		default:
			WARN(node, "unexpected member type (%d)\n", m->type);
			break;
		}
	}

	fprintf(fp, "}\n");

	// data
	fprintf(fp, " const __dt_%s_payload = {\n", node_ident);

	n_int = 0;
	n_reg = 0;
	n_base = 0;

	vector_for_each(&node->payload, m){
		switch(m->type){
		case MT_BASE_ADDR:	fprintf(fp, "\t.base%zu = (void*)%#x,\n", n_base++, m->payload); break;
		case MT_STRING:		fprintf(fp, "\t.string%zu = \"%s\",\n", n_base++, m->payload); break;

		case MT_REG_LIST:
			vector_for_each((vector_t*)m->payload, p)
				fprintf(fp, "\t.reg%zu = (void*)%#x,\n", n_reg++, *p);
			break;

		case MT_INT_LIST:
			int_lst = m->payload;

			vector_for_each(int_lst->lst, p)
				fprintf(fp, "\t.int%zu = %u,\n", n_int++, *p);
			break;

		default:
			// already reported above
			break;
		}
	}

	fprintf(fp, "};\n\n");
}
