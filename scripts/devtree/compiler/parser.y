/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



%define api.prefix {devtree}
%define parse.error verbose
%define parse.lac full
%locations

/* header */
%{
	#include <sys/escape.h>
	#include <stdarg.h>
	#include <stdio.h>
	#include <string.h>
	#include <asserts.h>
	#include <lexer.lex.h>
	#include <nodes.h>


	/* macros */
	#define YYDEBUG	1

	// parser error message
	#define EABORT(expr){ \
		if((expr) != 0) \
			YYERROR; \
	}

	// helper
	#define STRALLOC(s, len)({ \
		void *_s = stralloc(s, len); \
		EABORT(_s == 0x0); \
		_s; \
	})

	#define _CREATE(func, ...)({ \
		void *_obj = func(__VA_ARGS__); \
		EABORT(_obj == 0x0); \
		_obj; \
	})

	#define CREATE(type, ...) \
		_CREATE(type ## _create, ##__VA_ARGS__)

	#define _VALIDATE(func, ...) \
		EABORT(func(__VA_ARGS__) != 0)

	#define VALIDATE(type, ...) \
		_VALIDATE(type ## _validate, ##__VA_ARGS__)

	#define CHILD_ADD(node, child) \
		EABORT(node_child_add(node, child) != 0)

	#define ATTR_ADD(node, attr, v) \
		EABORT(node_attr_add(node, attr, v) != 0)

	#define ILIST_ADD(lst, v) \
		EABORT(ilist_add(lst, v) != 0);

	#define ASSERT_ADD(node, a) \
		node_assert_add(node, a)

	#define NODE_REF(idfr, type)({ \
		node_t *_node = node_ref(idfr.s, idfr.len, type); \
		EABORT(_node == 0x0); \
		_node; \
	})

	#define ATTR_REF(node, attr, idx)({ \
		attr_value_t *_v = node_attr_ref(node, attr, idx); \
		EABORT(_v == 0x0); \
		_v; \
	})

	#define ILIST_REF(node, attr, idx)({ \
		unsigned long int *_v = node_attr_ilist_ref(node, attr, idx); \
		EABORT(_v == 0x0); \
		_v; \
	})


	/* local/static variables */
	static FILE *fp = 0;
	static char const *dt_script = 0x0;


	/* prototypes */
	void devtreeunput(char c);


	/* local/static prototypes */
	static int devtreeerror(char const *file, char const *s);
	static void cleanup(void);
	static void *stralloc(char const *s, size_t len);
%}

%code requires{
	#include <sys/vector.h>
	#include <asserts.h>
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
	unsigned long int i;
	char *sptr;
	unsigned long int *iptr;
	attr_value_t *aptr;
	attr_type_t attr;

	struct{
		char *s;
		size_t len;
	} str;

	node_t *node;
	assert_t *assert;
	vector_t *vec;
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
%token NA_BASE_ADDR
%token NA_REG
%token NA_ADDR_WIDTH
%token NA_REG_WIDTH
%token NA_NCORES
%token NA_INTERRUPTS
%token NA_TIMER_CYCLE_TIME_US
%token NA_TIMER_INT
%token NA_SYSCALL_INT
%token NA_IPI_INT
%token NA_INT
%token NA_SIZE
%token NA_STRING

// asserts
%token ASSERT

/* non-terminals */
%type <node> device
%type <node> dev-body
%type <node> memory
%type <node> mem-body
%type <assert> assert
%type <attr> dev-attr-int
%type <attr> dev-attr-str
%type <attr> dev-attr-int-lst
%type <attr> dev-attr-reg-lst
%type <attr> arch-attr-int
%type <attr> mem-attr-int
%type <vec> ilist
%type <vec> opt-int
%type <i> int
%type <sptr> string
%type <iptr> i-ref
%type <aptr> iattr-ref
%type <aptr> sattr-ref
%type <iptr> ilist-ref
%type <i> attr-inc


%%


/* start */
start : error													{ cleanup(); YYABORT; }
	  | section-lst												{ VALIDATE(arch); memory_node_complement(memory_root()); cleanup(); }
	  ;

/* sections */
section-lst : %empty											{ }
			| section-lst section ';'							{ }
			;

section : SEC_DEVICES '=' '{' devices-lst '}'					{ }
		| SEC_MEMORY '=' '{' memory-lst '}'						{ }
		| SEC_ARCH '=' '{' arch-body '}'						{ }
		;

/* node lists */
devices-lst : %empty											{ }
			| devices-lst device ';'							{ CHILD_ADD(device_root(), $2); }
			| devices-lst assert ';'							{ ASSERT_ADD(device_root(), $2); }
			| devices-lst attr-update ';'						{ }
			;

memory-lst : %empty												{ }
		   | memory-lst memory ';'								{ CHILD_ADD(memory_root(), $2); }
		   | memory-lst assert ';'								{ ASSERT_ADD(memory_root(), $2); }
		   | memory-lst attr-update ';'							{ }
		   ;

/* nodes */
device : IDFR '=' '{' dev-body '}'								{ $$ = $4; $$->name = STRALLOC($1.s, $1.len); VALIDATE(device, $$); };
memory : IDFR '=' '{' mem-body '}'								{ $$ = $4; $$->name = STRALLOC($1.s, $1.len); VALIDATE(memory, $$); };

/* node bodies */
dev-body : %empty												{ $$ = CREATE(node, NT_DEVICE); }
		 | dev-body assert ';'									{ $$ = $1; ASSERT_ADD($$, $2); }
		 | dev-body attr-update ';'								{ $$ = $1; }
		 | dev-body device ';'									{ $$ = $1; CHILD_ADD($$, $2); }
		 | dev-body dev-attr-int '=' int ';'					{ $$ = $1; ATTR_ADD($$, $2, ATTR_VALUE(i, $4)); }
		 | dev-body dev-attr-str '=' string ';'					{ $$ = $1; ATTR_ADD($$, $2, ATTR_VALUE(p, $4)); }
		 | dev-body dev-attr-reg-lst '=' ilist ';'				{ $$ = $1; ATTR_ADD($$, MT_REG_LIST, ATTR_VALUE(lst, CREATE(attr_ilist, 0, $4))); }
		 | dev-body dev-attr-int-lst '<' int '>' '=' ilist ';'	{ $$ = $1; ATTR_ADD($$, MT_INT_LIST, ATTR_VALUE(lst, CREATE(attr_ilist, $4, $7))); }
		 ;

mem-body : %empty												{ $$ = CREATE(node, NT_MEMORY); }
		 | mem-body assert ';'									{ $$ = $1; ASSERT_ADD($$, $2); }
		 | mem-body attr-update ';'								{ $$ = $1; }
		 | mem-body memory ';'									{ $$ = $1; CHILD_ADD($$, $2); }
		 | mem-body mem-attr-int '=' int ';'					{ $$ = $1; ATTR_ADD($$, $2, ATTR_VALUE(i, $4)); }
		 ;

arch-body : %empty												{ }
		  | arch-body assert ';'								{ ASSERT_ADD(arch_root(), $2); }
		  | arch-body device ';'								{ CHILD_ADD(arch_root(), $2); }
		  | arch-body arch-attr-int '=' int ';'					{ ATTR_ADD(arch_root(), $2, ATTR_VALUE(i, $4)); }
		  ;

/* asserts */
assert : ASSERT '(' string ',' string ')'						{ $$ = CREATE(assert, $3, $5); };

/* references */
i-ref : iattr-ref												{ $$ = &$1->i; }
	  | ilist-ref												{ $$ = $1; }
	  ;

iattr-ref : IDFR '.' dev-attr-int '[' int ']'					{ $$ = ATTR_REF(NODE_REF($1, NT_DEVICE), $3, $5); }
		  | IDFR '.' mem-attr-int								{ $$ = ATTR_REF(NODE_REF($1, NT_MEMORY), $3, 0); }
		  | SEC_ARCH '.' arch-attr-int							{ $$ = ATTR_REF(arch_root(), $3, 0); }
		  ;

sattr-ref : IDFR '.' dev-attr-str '[' int ']'					{ $$ = ATTR_REF(NODE_REF($1, NT_DEVICE), $3, $5); }
		  | IDFR '.' dev-attr-str								{ $$ = ATTR_REF(NODE_REF($1, NT_DEVICE), $3, 0); }
		  ;

ilist-ref : IDFR '.' dev-attr-int-lst '[' int ']'				{ $$ = ILIST_REF(NODE_REF($1, NT_DEVICE), $3, $5); }
		  ;

/* attribute updates */
attr-update : i-ref '=' int										{ *$1 = $3; }
			| i-ref '+' '=' int									{ *$1 += $4; }
			| sattr-ref '=' string								{ $1->p = $3; }
			| attr-inc											{ }
			;

attr-inc : i-ref '+' '+'										{ $$ = (*$1)++; }
		 | '+' '+' i-ref										{ $$ = ++(*$3); }
		 ;

/* basic types */
ilist : '[' opt-int ']'											{ $$ = $2; }
	  | '[' opt-int ',' ']'										{ $$ = $2; }
	  ;

opt-int : %empty												{ $$ = CREATE(ilist); devtreeunput(','); }
		| opt-int ',' int										{ $$ = $1; ILIST_ADD($$, $3); }
		;

int : INT														{ $$ = $1; }
	| i-ref														{ $$ = *$1; }
	| int '+' INT												{ $$ = $1 + $3; }
	| int '+' i-ref												{ $$ = $1 + *$3; }
	| '(' attr-inc ')'											{ $$ = $2; }
	;

string : STRING													{ $$ = STRALLOC($1.s, $1.len); }
	   | sattr-ref												{ $$ = $1->p; }
	   ;

/* node attributes */
dev-attr-int : NA_BASE_ADDR										{ $$ = MT_BASE_ADDR; }
			 ;

dev-attr-str : NA_COMPATIBLE									{ $$ = MT_COMPATIBLE; }
			 | NA_STRING										{ $$ = MT_STRING; }
			 ;

dev-attr-int-lst : NA_INT										{ $$ = MT_INT_LIST; }
				 ;

dev-attr-reg-lst : NA_REG										{ $$ = MT_REG_LIST; }
				 ;

mem-attr-int : NA_BASE_ADDR										{ $$ = MT_BASE_ADDR; }
			 | NA_SIZE											{ $$ = MT_SIZE; }
			 ;

arch-attr-int : NA_ADDR_WIDTH									{ $$ = MT_ADDR_WIDTH; }
			  | NA_REG_WIDTH									{ $$ = MT_REG_WIDTH; }
			  | NA_NCORES										{ $$ = MT_NCORES; }
			  | NA_INTERRUPTS									{ $$ = MT_NUM_INTS; }
			  | NA_TIMER_INT									{ $$ = MT_TIMER_INT; }
			  | NA_SYSCALL_INT									{ $$ = MT_SYSCALL_INT; }
			  | NA_IPI_INT										{ $$ = MT_IPI_INT; }
			  | NA_TIMER_CYCLE_TIME_US							{ $$ = MT_TIMER_CYCLE_TIME_US; }
			  ;


%%


/* global functions */
int devtree_parser_error(char const *fmt, ...){
	va_list lst;


	fprintf(stderr, FG("%s", PURPLE) ":" FG("%d:%d", GREEN) " token \"%s\" -- ",
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
