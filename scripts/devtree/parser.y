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
	#include <stdio.h>
	#include <limits.h>
	#include <string.h>
	#include <sys/compiler.h>
	#include <lexer.lex.h>
	#include <node.h>


	/* macros */
	// extended error messages
	#define YYERROR_VERBOSE
	#define YYDEBUG	1

	// node allocate
	#define NODE_ALLOC_DEVICES()({ \
		device_node_t *n; \
		\
		\
		if((n = calloc(1, sizeof(device_node_t))) == 0x0) \
			PARSER_ERROR("allocating node\n"); \
		\
		if(vector_init(&n->payload, sizeof(member_t), 16) != 0) \
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
	#define NODE_VALIDATE_DEVICES(node)({ \
		char const **name; \
		\
		\
		if(node->compatible == 0x0 || *node->compatible == 0) \
			PARSER_ERROR("undefined member \"compatible\"\n"); \
		\
		vector_for_each(&node_names, name){ \
			if(strcmp(*name, (node)->name) == 0) \
				PARSER_ERROR("node with name \"%s\" already exists\n", *name); \
		} \
		\
		vector_add(&node_names, &((node)->name)); \
	})

	#define NODE_VALIDATE_MEMORY(node)({ \
		char const **name; \
		\
		\
		if(node->size == 0 && node->childs == 0x0) \
			PARSER_ERROR("zero size for node \"%s\"\n", node->name); \
		\
		vector_for_each(&node_names, name){ \
			if(strcmp(*name, (node)->name) == 0) \
				PARSER_ERROR("node with name \"%s\" already exists\n", *name); \
		} \
		\
		vector_add(&node_names, &((node)->name)); \
	})

	// vector allocate
	#define INT_LIST_ALLOC()({ \
		vector_t *v; \
		\
		\
		if((v = malloc(sizeof(vector_t))) == 0x0) \
			PARSER_ERROR("allocating vector\n"); \
		\
		if(vector_init(v, sizeof(unsigned int), 16) != 0) \
			PARSER_ERROR("init vector\n"); \
		\
		v; \
	})

	// vector add
	#define INT_LIST_ADD(v, payload){ \
		if(!payload.empty && vector_add(v, &payload.val) != 0) \
			PARSER_ERROR("adding to vector\n"); \
	}


	// check if a member is already present
	#define MEMBER_PRESENT(node, member, empty_val)({ \
		if((node)->member != (empty_val)) \
			PARSER_ERROR("member \"" STR(member) "\" already set\n"); \
	})

	// allocate integer member
	#define MEMBER_ALLOC_INTLIST(int_size, payload)({ \
		member_int_t *l; \
		\
		\
		if((l = malloc(sizeof(member_int_t))) == 0x0) \
			PARSER_ERROR("allocating integer list\n"); \
		\
		l->size = (int_size); \
		l->lst = (payload); \
		l; \
	})

	// add member to node
	#define MEMBER_ADD(node, mem_type, val){ \
		member_t m; \
		\
		\
		m.type = (mem_type); \
		m.payload = (val); \
		\
		if(vector_add(&(node)->payload, &m) != 0) \
			PARSER_ERROR("adding member \"" STR(mem_type) "\"\n"); \
	}

	// string alloc
	#define STRALLOC(_s, len)({ \
		char *s; \
		\
		\
		if(len + 1 > NAME_MAX) \
			PARSER_ERROR("identifier \"%*.*s\" too long, maximum length %u\n", len, len, _s, NAME_MAX); \
		\
		s = malloc(len + 1); \
		\
		if(s == 0x0) \
			PARSER_ERROR("out of memory allocating string\n"); \
		\
		memcpy(s, _s, len); \
		s[len] = 0; \
		\
		s; \
	})

	// parser error message
	#define PARSER_ERROR(s, ...){ \
		fprintf(stderr, FG_VIOLETT "%s" RESET_ATTR ":" FG_GREEN "%d:%d" RESET_ATTR " token \"%s\" -- " s, file, devtreelloc.first_line, devtreelloc.first_column, devtreetext, ##__VA_ARGS__); \
		YYERROR; \
	}

	/* local/static variables */
	static FILE *fp = 0;
	static vector_t node_names;


	/* prototypes */
	static int devtreeerror(char const *file, device_node_t *devices_root, memory_node_t *memory_root, char const *s);
	static void cleanup(void);
%}

%code requires{
	#include <sys/escape.h>
	#include <sys/list.h>
	#include <sys/vector.h>
	#include <node.h>
}

/* parse paramters */
%parse-param { char const *file }
%parse-param { device_node_t *devices_root }
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
		bool empty;
		unsigned int val;
	} opt_int;

	struct{
		char *s;
		unsigned int len;
	} str;

	device_node_t *devices;
	memory_node_t *memory;
	vector_t *int_lst;
}

/* terminals */
// general
%token <i> INT
%token <str> STRING
%token <str> IDFR

// sections
%token SEC_DEVICES
%token SEC_MEMORY

// node attributes
%token NA_COMPATIBLE
%token NA_BASEADDR
%token NA_REG
%token NA_INT
%token NA_SIZE
%token NA_STRING

/* non-terminals */
%type <devices> devices
%type <devices> devices-attr
%type <memory> memory
%type <memory> memory-attr
%type <int_lst> int-list
%type <opt_int> int-list-val


%%


/* start */
start : error															{ cleanup(); YYABORT; }
	  | section-lst														{ cleanup(); }
	  ;

/* sections */
section-lst : %empty													{ }
			| section-lst section ';'									{ }
			;

section : SEC_DEVICES '=' '{' devices-lst '}'							{ }
		| SEC_MEMORY '=' '{' memory-lst '}'								{ }
		;

/* node lists */
devices-lst : %empty { } | devices-lst devices ';'						{ NODE_VALIDATE_DEVICES($2); $2->parent = devices_root; list_add_tail(devices_root->childs, $2); };
memory-lst : %empty { } | memory-lst memory ';'							{ NODE_VALIDATE_MEMORY($2); $2->parent = memory_root; list_add_tail(memory_root->childs, $2); };

/* nodes */
devices : IDFR '=' '{' devices-attr '}'									{ $$ = $4; $$->name = STRALLOC($1.s, $1.len); };
memory : IDFR '=' '{' memory-attr '}'									{ $$ = $4; $$->name = STRALLOC($1.s, $1.len); };

/* node attributes */
devices-attr : %empty													{ $$ = NODE_ALLOC_DEVICES(); }
			| devices-attr devices ';'									{ $$ = $1; NODE_VALIDATE_DEVICES($2); $2->parent = $$; list_add_tail($$->childs, $2); }
			| devices-attr NA_COMPATIBLE '=' STRING ';'					{ $$ = $1; MEMBER_PRESENT($$, compatible, 0x0); $$->compatible = STRALLOC($4.s, $4.len); }
			| devices-attr NA_BASEADDR '=' INT ';'						{ $$ = $1; MEMBER_ADD($$, MT_BASE_ADDR, (void*)(unsigned long int)$4); }
			| devices-attr NA_REG '=' '[' int-list ']' ';'				{ $$ = $1; MEMBER_ADD($$, MT_REG_LIST, $5); }
			| devices-attr NA_INT '<' INT '>' '=' '[' int-list ']' ';'	{ $$ = $1; MEMBER_ADD($$, MT_INT_LIST, MEMBER_ALLOC_INTLIST($4, $8)); }
			| devices-attr NA_STRING '=' STRING ';'						{ $$ = $1; MEMBER_ADD($$, MT_STRING, STRALLOC($4.s, $4.len)); }
			;

memory-attr : %empty													{ $$ = NODE_ALLOC_MEMORY(); }
			| memory-attr memory ';'									{ $$ = $1; NODE_VALIDATE_MEMORY($2); $2->parent = $$; list_add_tail($$->childs, $2); }
			| memory-attr NA_BASEADDR '=' INT ';'						{ $$ = $1; $$->base = (void*)(unsigned long int)$4; }
			| memory-attr NA_SIZE '=' INT ';'							{ $$ = $1; $$->size = (size_t)$4; }
			;

/* basic types */
int-list : int-list-val													{ $$ = INT_LIST_ALLOC(); INT_LIST_ADD($$, $1); }
		 | int-list opt-com int-list-val								{ $$ = $1; INT_LIST_ADD($$, $3); }
		 ;

int-list-val : %empty													{ $$.empty = true; }
			 | INT														{ $$.empty = false; $$.val = $1; }
			 ;

opt-com : %empty														{ }
		| ','															{ }
		;


%%


static int devtreeerror(char const *file, device_node_t *devices_root, memory_node_t *memory_root, char const *s){
	PARSER_ERROR("%s\n", s);

yyerrorlab:
	return 0;
}

static void cleanup(void){
	devtreelex_destroy();
	vector_destroy(&node_names);
	fclose(fp);
}
