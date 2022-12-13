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
	#include <sys/escape.h>
	#include <stdio.h>
	#include <stdarg.h>
	#include <limits.h>
	#include <string.h>
	#include <lexer.lex.h>
	#include <nodes.h>


	/* macros */
	// extended error messages
	#define YYERROR_VERBOSE
	#define YYDEBUG	1

	// parser error message
	#define EABORT(expr){ \
		if((expr) != 0) \
			YYERROR; \
	}

	// helper
	#define NODE_ADD_CHILD(parent, child)	node_add_child((base_node_t*)parent, (base_node_t*)child)


	/* local/static variables */
	static FILE *fp = 0;
	static char const *dt_script = 0x0;


	/* prototypes */
	void devtreeunput(char c);


	/* local/static prototypes */
	static int devtreeerror(char const *file, char const *s);

	static void cleanup(void);

	static void *stralloc(char const *s, size_t len);
	static vector_t *intlist_alloc(void);
	static int intlist_add(vector_t *lst, unsigned int val);
%}

%code requires{
	#include <sys/vector.h>
	#include <nodes.h>


	/* prototypes */
	int devtree_parser_error(char const *fmt, ...);
}

/* parse paramters */
%parse-param { char const *file }

/* init code */
%initial-action{
	/* open input file */
	dt_script = file;
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

	device_node_t *device;
	memory_node_t *memory;
	arch_node_t *arch;
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
%token SEC_ARCH

// node attributes
%token NA_COMPATIBLE
%token NA_BASEADDR
%token NA_REG
%token NA_ADDR_WIDTH
%token NA_REG_WIDTH
%token NA_INTERRUPTS
%token NA_VIRTUAL_INTERRUPTS
%token NA_TIMER_CYCLE_TIME_US
%token NA_TIMER_INT
%token NA_INT
%token NA_SIZE
%token NA_STRING

/* non-terminals */
%type <device> device
%type <device> devices-attr
%type <memory> memory
%type <memory> memory-attr
%type <i> int
%type <int_lst> int-list
%type <int_lst> opt-int


%%


/* start */
start : error														{ cleanup(); YYABORT; }
	  | section-lst													{ EABORT(arch_validate()); memory_node_complement(memory_root()); cleanup(); }
	  ;

/* sections */
section-lst : %empty												{ }
			| section-lst section ';'								{ }
			;

section : SEC_DEVICES '=' '{' devices-lst '}'						{ }
		| SEC_MEMORY '=' '{' memory-lst '}'							{ }
		| SEC_ARCH '=' '{' arch-lst '}'								{ }
		;

/* node lists */
devices-lst : %empty												{ }
			| devices-lst device ';'								{ EABORT(NODE_ADD_CHILD(device_root(), $2)); };

memory-lst : %empty													{ }
		   | memory-lst memory ';'									{ EABORT(NODE_ADD_CHILD(memory_root(), $2)); };

arch-lst : %empty													{ }
		 | arch-lst device ';'										{ EABORT(NODE_ADD_CHILD(arch_root(), $2)); }
		 | arch-lst NA_ADDR_WIDTH '=' int ';'						{ arch_root()->addr_width = $4; }
		 | arch-lst NA_REG_WIDTH '=' int ';'						{ arch_root()->reg_width = $4; }
		 | arch-lst NA_INTERRUPTS '=' int ';'						{ arch_root()->num_ints = $4; }
		 | arch-lst NA_VIRTUAL_INTERRUPTS '=' int ';'				{ arch_root()->num_vints = $4; }
		 | arch-lst NA_TIMER_CYCLE_TIME_US '=' int ';'				{ arch_root()->timer_cycle_time_us = $4; }
		 | arch-lst NA_TIMER_INT '=' int ';'						{ arch_root()->timer_int = $4; }
		 ;

/* nodes */
device : IDFR '=' '{' devices-attr '}'								{ $$ = $4; $$->name = stralloc($1.s, $1.len); EABORT($$->name == 0x0); };
memory : IDFR '=' '{' memory-attr '}'								{ $$ = $4; $$->name = stralloc($1.s, $1.len); EABORT($$->name == 0x0); };

/* node attributes */
devices-attr : %empty												{ $$ = device_node_alloc(); EABORT($$ == 0x0); }
			 | devices-attr device ';'								{ $$ = $1; EABORT(NODE_ADD_CHILD($$, $2)); }
			 | devices-attr NA_COMPATIBLE '=' STRING ';'			{ $$ = $1; $$->compatible = stralloc($4.s, $4.len); EABORT($$->compatible == 0x0); }
			 | devices-attr NA_BASEADDR '=' int ';'					{ $$ = $1; EABORT(device_node_add_member($$, MT_BASE_ADDR, (void*)(unsigned long int)$4)); }
			 | devices-attr NA_REG '=' int-list ';'					{ $$ = $1; EABORT(device_node_add_member($$, MT_REG_LIST, $4)); }
			 | devices-attr NA_INT '<' int '>' '=' int-list ';'		{ $$ = $1; EABORT(device_node_add_member($$, MT_INT_LIST, node_intlist_alloc($4, $7))); }
			 | devices-attr NA_STRING '=' STRING ';'				{ $$ = $1; EABORT(device_node_add_member($$, MT_STRING, stralloc($4.s, $4.len))); }
			 ;

memory-attr : %empty												{ $$ = memory_node_alloc(); EABORT($$ == 0x0); }
			| memory-attr memory ';'								{ $$ = $1; EABORT(NODE_ADD_CHILD($$, $2)); }
			| memory-attr NA_BASEADDR '=' int ';'					{ $$ = $1; $$->base = (void*)(unsigned long int)$4; }
			| memory-attr NA_SIZE '=' int ';'						{ $$ = $1; $$->size = (size_t)$4; }
			;

/* basic types */
int-list : '[' opt-int ']'											{ $$ = $2; }
		 | '[' opt-int ',' ']'										{ $$ = $2; }
		 ;

opt-int : %empty													{ $$ = intlist_alloc(); EABORT($$ == 0x0); devtreeunput(','); }
		| opt-int ',' int											{ $$ = $1; EABORT(intlist_add($$, $3)); }
		;

int : INT															{ $$ = $1; }
	| int '+' INT													{ $$ += $3; }
	;


%%


/* global functions */
int devtree_parser_error(char const *fmt, ...){
	va_list lst;


	fprintf(stderr, FG_VIOLETT "%s" RESET_ATTR ":" FG_GREEN "%d:%d" RESET_ATTR " token \"%s\" -- ",
		dt_script,
		devtreelloc.first_line,
		devtreelloc.first_column,
		devtreetext
	);

	va_start(lst, fmt);
	vfprintf(stderr, fmt, lst);
	va_end(lst);

	fprintf(stderr, " %s\n", (errno ? strerror(errno) : ""));

	return -1;
}


/* local functions */
static int devtreeerror(char const *file, char const *s){
	devtree_parser_error(s);

	return 0;
}

static void cleanup(void){
	devtreelex_destroy();
	fclose(fp);
}

static void *stralloc(char const *s, size_t len){
	char *x;


	x = malloc(len + 1);

	if(x == 0x0)
		goto err;

	memcpy(x, s, len);
	x[len] = 0;

	return x;


err:
	devtree_parser_error("string allocation failed");

	return 0x0;
}

static vector_t *intlist_alloc(void){
	vector_t *v;


	v = malloc(sizeof(vector_t));

	if(v == 0x0)
		goto err_0;

	if(vector_init(v, sizeof(unsigned int), 16) != 0)
		goto err_1;

	return v;


err_1:
	free(v);

err_0:
	devtree_parser_error("intlist allocation failed");

	return 0x0;
}

static int intlist_add(vector_t *lst, unsigned int val){
	if(vector_add(lst, &val) != 0)
		return devtree_parser_error("intlist extension failed");

	return 0;
}
