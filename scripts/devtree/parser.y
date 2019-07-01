/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



%define api.prefix {devtree}
%locations

/* header */
%{
	#include <sys/compiler.h>
	#include <lexer.lex.h>
	#include <node.h>
	#include <stdio.h>
	#include <string.h>


	/* macros */
	// extended error messages
	#define YYERROR_VERBOSE
	#define YYDEBUG	1

	// node allocate
	#define NODE_ALLOC_DRIVER()({ \
		driver_node_t *n; \
		\
		\
		if((n = calloc(1, sizeof(driver_node_t))) == 0x0) \
			PARSER_ERROR("allocating node\n"); \
		\
		if(vector_init(&n->data, sizeof(member_t), 16) != 0) \
			PARSER_ERROR("init member vector\n"); \
		\
		n; \
	})

	#define NODE_ALLOC_MEMORY()({ \
		memory_node_t *n; \
		\
		\
		if((n = calloc(1, sizeof(memory_node_t))) == 0x0) \
			PARSER_ERROR("allocating node\n"); \
		\
		n; \
	})

	// node validation
	#define NODE_VALIDATE_DRIVER(node)({ \
		char const *c; \
		\
		\
		if(node->compatible == 0x0 || *node->compatible == 0) \
			PARSER_ERROR("undefined member \"compatible\"\n"); \
		\
		vector_for_each(&node_names, c){ \
			if(strcmp(c, (node)->name) == 0) \
				PARSER_ERROR("node with name \"%s\" already exists\n", c); \
		} \
		\
		vector_add(&node_names, (char*)(node)->name); \
	})

	#define NODE_VALIDATE_MEMORY(node)({ \
		char const *c; \
		\
		\
		if(node->size == 0 && node->childs == 0x0) \
			PARSER_ERROR("zero size for node \"%s\"\n", node->name); \
		\
		vector_for_each(&node_names, c){ \
			if(strcmp(c, (node)->name) == 0) \
				PARSER_ERROR("node with name \"%s\" already exists\n", c); \
		} \
		\
		vector_add(&node_names, (char*)(node)->name); \
	})

	// vector allocate
	#define VECTOR_ALLOC(data)({ \
		vector_t *v; \
		\
		\
		if((v = malloc(sizeof(vector_t))) == 0x0) \
			PARSER_ERROR("allocating vector\n"); \
		\
		if(vector_init(v, sizeof(unsigned int), 16) != 0) \
			PARSER_ERROR("init vector\n"); \
		\
		VECTOR_ADD(v, data); \
		\
		v; \
	})

	// vector add
	#define VECTOR_ADD(v, data){ \
		if(vector_add(v, data) != 0) \
			PARSER_ERROR("adding to vector\n"); \
		\
	}


	// check if a member is already present
	#define MEMBER_PRESENT(node, member, empty_val)({ \
		if((node)->member != (empty_val)) \
			PARSER_ERROR("member \"" STR(member) "\" already set\n"); \
	})

	// allocate integer member
	#define MEMBER_ALLOC_INTLIST(int_size, data)({ \
		member_int_t *l; \
		\
		\
		if((l = malloc(sizeof(member_int_t))) == 0x0) \
			PARSER_ERROR("allocating integer list\n"); \
		\
		l->size = (int_size); \
		l->lst = (data); \
		l; \
	})

	// add member to node
	#define MEMBER_ADD(node, mem_type, val){ \
		member_t m; \
		\
		\
		m.type = (mem_type); \
		m.data = (val); \
		\
		if(vector_add(&(node)->data, &m) != 0) \
			PARSER_ERROR("adding member \"" STR(mem_type) "\"\n"); \
	}

	// parser error message
	#define PARSER_ERROR(s, ...){ \
		fprintf(stderr, FG_VIOLETT "%s" RESET_ATTR ":" FG_GREEN "%d:%d " RESET_ATTR "error " s, file, devtreelloc.first_line, devtreelloc.first_column, ##__VA_ARGS__); \
		YYERROR; \
	}

	/* local/static variables */
	static FILE *fp = 0;
	static vector_t node_names;


	/* prototypes */
	static int devtreeerror(char const *file, driver_node_t *driver_root, memory_node_t *memory_root, char const *s);
	static void cleanup(void);
	static char *stralloc(char *s, size_t len);
%}

%code requires{
	#include <sys/escape.h>
	#include <sys/list.h>
	#include <sys/vector.h>
	#include <node.h>
}

/* parse paramters */
%parse-param { char const *file }
%parse-param { driver_node_t *driver_root }
%parse-param { memory_node_t *memory_root }

/* init code */
%initial-action{
	/* init node name list */
	vector_init(&node_names, sizeof(char*), 16);

	/* open input file */
	fp = fopen(file, "r");

	if(fp == 0){
		fprintf(stderr, "read config file \"%s\" failed \"%s\"\n", file, strerror(errno));
		return 1;
	}

	/* start lexer */
	devtreerestart(fp);
}

/* parser union type */
%union{
	unsigned int i;

	struct{
		char *s;
		unsigned int len;
	} str;

	driver_node_t *driver;
	memory_node_t *memory;
	vector_t *int_lst;
}

/* terminals */
// general
%token <i> INT
%token <str> STRING
%token <str> IDFR

// sections
%token SEC_DRIVER
%token SEC_MEMORY

// node attributes
%token NA_COMPATIBLE
%token NA_BASEADDR
%token NA_REG
%token NA_INT
%token NA_SIZE

/* non-terminals */
%type <driver> driver
%type <driver> driver-attr
%type <memory> memory
%type <memory> memory-attr
%type <int_lst> int-list


%%


/* start */
start : error															{ cleanup(); YYABORT; }
	  | section-lst														{ cleanup(); }
	  ;

/* sections */
section-lst : %empty													{ }
			| section-lst section ';'									{ }
			;

section : SEC_DRIVER '=' '{' driver-lst '}'								{ }
		| SEC_MEMORY '=' '{' memory-lst '}'								{ }
		;

/* node lists */
driver-lst : %empty { } | driver-lst driver ';'							{ NODE_VALIDATE_DRIVER($2); $2->parent = driver_root; list_add_tail(driver_root->childs, $2); };
memory-lst : %empty { } | memory-lst memory ';'							{ NODE_VALIDATE_MEMORY($2); $2->parent = memory_root; list_add_tail(memory_root->childs, $2); };

/* nodes */
driver : IDFR '=' '{' driver-attr '}'									{ $$ = $4; $$->name = stralloc($1.s, $1.len); };
memory : IDFR '=' '{' memory-attr '}'									{ $$ = $4; $$->name = stralloc($1.s, $1.len); };

/* node attributes */
driver-attr : %empty													{ $$ = NODE_ALLOC_DRIVER(); }
			| driver-attr driver ';'									{ $$ = $1; NODE_VALIDATE_DRIVER($2); $2->parent = $$; list_add_tail($$->childs, $2); }
			| driver-attr NA_COMPATIBLE '=' STRING ';'					{ $$ = $1; MEMBER_PRESENT($$, compatible, 0x0); $$->compatible = stralloc($4.s, $4.len); }
			| driver-attr NA_BASEADDR '=' INT ';'						{ $$ = $1; MEMBER_ADD($$, MT_BASE_ADDR, (void*)(unsigned long int)$4); }
			| driver-attr NA_REG '=' '[' int-list ']' ';'				{ $$ = $1; MEMBER_ADD($$, MT_REG_LIST, $5); }
			| driver-attr NA_INT '<' INT '>' '=' '[' int-list ']' ';'	{ $$ = $1; MEMBER_ADD($$, MT_INT_LIST, MEMBER_ALLOC_INTLIST($4, $8)); }
			;

memory-attr : %empty													{ $$ = NODE_ALLOC_MEMORY(); }
			| memory-attr memory ';'									{ $$ = $1; NODE_VALIDATE_MEMORY($2); $2->parent = $$; list_add_tail($$->childs, $2); }
			| memory-attr NA_BASEADDR '=' INT ';'						{ $$ = $1; $$->base = (void*)(unsigned long int)$4; }
			| memory-attr NA_SIZE '=' INT ';'							{ $$ = $1; $$->size = (size_t)$4; }
			;

/* basic types */
int-list : INT															{ $$ = VECTOR_ALLOC(&$1); }
		 | int-list ',' INT												{ $$ = $1; VECTOR_ADD($$, &$3); }
		 ;


%%


static int devtreeerror(char const *file, driver_node_t *driver_root, memory_node_t *memory_root, char const *s){
	fprintf(stderr, FG_VIOLETT "%s" RESET_ATTR ":" FG_GREEN "%d:%d" RESET_ATTR " token \"%s\" -- %s\n", file, devtreelloc.first_line, devtreelloc.first_column, devtreetext, s);
	return 0;
}

static void cleanup(void){
	devtreelex_destroy();
	vector_destroy(&node_names);
	fclose(fp);
}

static char *stralloc(char *_s, size_t len){
	char *s;


	s = malloc(len);

	if(s != 0x0)
		memcpy(s, _s, len);

	return s;
}
