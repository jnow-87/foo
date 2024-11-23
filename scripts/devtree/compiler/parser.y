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
	#include <types.h>


	/* macros */
	#define YYDEBUG	1

	// parser error message
	#define EABORT(expr){ \
		if((expr) != 0) \
			YYERROR; \
	}

	// helper
	#define STRALLOC(s)({ \
		void *_s = stralloc(s); \
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

	#define CHILD_ADD(node, child) \
		EABORT(node_child_add(node, child) != 0)

	#define ATTR_ADD(attrs, name, type, v) \
		EABORT(attr_add(attrs, name, type, v) != 0)

	#define NODE_REF(idfr)({ \
		node_t *_node = node_ref(idfr); \
		EABORT(_node == 0x0); \
		_node; \
	})

	#define ATTR_REF(attrs, name)({ \
		attr_t *_attr = attr_get(attrs, name, false); \
		EABORT(_attr == 0x0); \
		_attr; \
	})

	#define ATTR_TYPE_CHECK(attr, type) \
		EABORT(attr_type_check(attr, type) != 0)

	#define TYPE_LOOKUP(name)({ \
		type_t *_type = type_lookup(name); \
		EABORT(_type == 0x0); \
		_type; \
	})

	#define OBJECT_RESET(obj){ \
		(obj).attrs = VECTOR_INITIALISER(sizeof(attr_t)); \
		(obj).asserts = 0x0; \
		(obj).childs = 0x0; \
	}


	/* local/static variables */
	static FILE *fp = 0;
	static char const *dt_script = 0x0;


	/* prototypes */
	void devtreeunput(char c);


	/* local/static prototypes */
	static int devtreeerror(char const *file, char const *s);
	static void cleanup(void);
	static void *stralloc(char const *s);
%}

%code requires{
	#include <sys/list.h>
	#include <sys/vector.h>
	#include <asserts.h>
	#include <nodes.h>
	#include <types.h>


	/* macros */
	#define DEVTREE_STRMAX	64


	/* prototypes */
	int devtree_parser_error(char const *fmt, ...);
	int devtree_parser_strcpy(char *dst, char const *src, size_t n, int token);
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
	char str[DEVTREE_STRMAX];
	char *sptr;
	unsigned long int *iptr;
	attr_t *attr;
	attr_type_t attr_type;

	node_t *node;
	assert_t *assert;

	struct{
		vector_t attrs;
		assert_t *asserts;
		node_t *childs;
	} object;
}

/* terminals */
// general
%token <i> INT
%token <str> STRING
%token <str> IDFR

// typedef
%token TYPEDEF_MEM
%token TYPEDEF_DEV
%token TYPEDEF_ARCH

// sections
%token SEC_ARCH
%token SEC_MEMORY
%token SEC_DEVICES

// node attributes
%token NA_ADDR
%token NA_INT8
%token NA_INT16
%token NA_INT32
%token NA_INT64
%token NA_STRING

// asserts
%token ASSERT

/* non-terminals */
%type <object> type-body
%type <object> type-args
%type <node> device
%type <assert> assert
%type <attr_type> sattr
%type <attr_type> iattr
%type <i> int
%type <sptr> string
%type <attr> attr-ref
%type <i> attr-inc


%%


/* start */
start : error													{ cleanup(); YYABORT; }
	  | section-lst												{ cleanup(); }
	  ;

/* sections */
section-lst : %empty											{ }
			| section-lst ';'									{ }
			| section-lst typedef ';'							{ }
			| section-lst device ';'							{ CHILD_ADD(nodes_root(), $2); }
			| section-lst attr-update ';'						{ }
			;

/* typedef */
typedef : TYPEDEF_ARCH '{' type-body '}' IDFR					{ EABORT(type_add(STRALLOC($5), TC_ARCH, &$3.attrs, $3.asserts)); }
		| TYPEDEF_MEM '{' type-body '}' IDFR					{ EABORT(type_add(STRALLOC($5), TC_MEMORY, &$3.attrs, $3.asserts)); }
		| TYPEDEF_DEV '{' type-body '}' IDFR					{ EABORT(type_add(STRALLOC($5), TC_DEVICE, &$3.attrs, $3.asserts)); }
		;

type-body : %empty											{ OBJECT_RESET($$); }
		  | type-body ';'									{ }
		  | type-body assert ';'							{ $$ = $1; list_add_tail($$.asserts, $2); }
		  | type-body sattr IDFR ';'						{ $$ = $1; attr_add(&$$.attrs, STRALLOC($3), $2, 0x0); }
		  | type-body sattr IDFR '=' string ';'				{ $$ = $1; attr_add(&$$.attrs, STRALLOC($3), $2, &ATTR_VALUE(p, $5)); }
		  | type-body iattr IDFR ';'						{ $$ = $1; attr_add(&$$.attrs, STRALLOC($3), $2, 0x0); }
		  | type-body iattr IDFR '=' int ';'				{ $$ = $1; attr_add(&$$.attrs, STRALLOC($3), $2, &ATTR_VALUE(i, $5)); }
		  | type-body iattr IDFR '=' attr-ref ';'			{ $$ = $1; ATTR_TYPE_CHECK($5, $2); attr_add(&$$.attrs, STRALLOC($3), $2, &ATTR_VALUE(i, $5->value.i)); }
		  | type-body sattr IDFR '=' attr-ref ';'			{ $$ = $1; ATTR_TYPE_CHECK($5, $2); attr_add(&$$.attrs, STRALLOC($3), $2, &ATTR_VALUE(p, $5->value.p)); }
		  ;

/* nodes */
device : IDFR '=' IDFR '(' type-args ')'						{ $$ = type_instantiate(TYPE_LOOKUP($3), STRALLOC($1), &$5.attrs, $5.childs); EABORT($$ == 0x0); }
	   | IDFR '=' IDFR '(' type-args ',' ')'					{ $$ = type_instantiate(TYPE_LOOKUP($3), STRALLOC($1), &$5.attrs, $5.childs); EABORT($$ == 0x0); }
	   ;

/* node bodies */
type-args : %empty												{ OBJECT_RESET($$); devtreeunput(','); }
		  | type-args ',' device								{ $$ = $1; list_add_tail($$.childs, $3); }
		  | type-args ',' IDFR '=' int							{ $$ = $1; ATTR_ADD(&$$.attrs, STRALLOC($3), MT_INT, &ATTR_VALUE(i, $5)); }
		  | type-args ',' IDFR '=' string						{ $$ = $1; ATTR_ADD(&$$.attrs, STRALLOC($3), MT_STRING, &ATTR_VALUE(p, $5)); }
		  | type-args ',' IDFR '=' attr-ref						{ $$ = $1; ATTR_ADD(&$$.attrs, STRALLOC($3), $5->type, &$5->value); }
		  ;

/* asserts */
assert : ASSERT '(' string ',' string ')'						{ $$ = CREATE(assert, $3, $5); };

/* references */
attr-ref : IDFR '.' IDFR								{ $$ = ATTR_REF(&NODE_REF($1)->attrs, $3); };

/* attribute updates */
attr-update : attr-ref '=' int									{ ATTR_TYPE_CHECK($1, MT_INT); $1->value.i = $3; }
			| attr-ref '+' '=' int								{ ATTR_TYPE_CHECK($1, MT_INT); $1->value.i += $4; }
			| attr-ref '=' string								{ ATTR_TYPE_CHECK($1, MT_STRING); $1->value.p = $3; }
			| attr-ref '=' attr-ref									{ ATTR_TYPE_CHECK($1, $3->type); $1->value = $3->value; }
			| attr-ref '+' '=' attr-ref								{ ATTR_TYPE_CHECK($1, MT_INT); ATTR_TYPE_CHECK($4, MT_INT); $1->value.i += $4->value.i; }
			| attr-inc											{ }
			;

attr-inc : attr-ref '+' '+'										{ ATTR_TYPE_CHECK($1, MT_INT); $$ = ($1->value.i)++; }
		 | '+' '+' attr-ref										{ ATTR_TYPE_CHECK($3, MT_INT); $$ = ++($3->value.i); }
		 ;

/* basic types */
int : INT													{ $$ = $1; }
	| int '+' INT											{ $$ = $1 + $3; }
	| int '+' attr-ref										{ ATTR_TYPE_CHECK($3, MT_INT); $$ = $1 + $3->value.i; }
	| '(' attr-inc ')'										{ $$ = $2; }
	;

string : STRING												{ $$ = STRALLOC($1); }
	   ;

/* node attributes */
sattr : NA_STRING										{ $$ = MT_STRING; };

iattr : NA_INT8   										{ $$ = MT_INT8; }
	  | NA_INT16										{ $$ = MT_INT16; }
	  | NA_INT32										{ $$ = MT_INT32; }
	  | NA_INT64										{ $$ = MT_INT64; }
	  | NA_ADDR											{ $$ = MT_ADDR; };
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

int devtree_parser_strcpy(char *dst, char const *src, size_t n, int token){
	if(n >= DEVTREE_STRMAX){
		devtree_parser_error("string too long, max=%u", DEVTREE_STRMAX);

		// trigger a parser error
		// this is not the nicest way to trigger an error since it will cause a syntax
		// error even though it is not a syntax error and therefor confuse the user
		return YYSYMBOL_YYEOF;
	}

	strncpy(dst, src, n);
	dst[n] = 0;

	return token;
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

static void *stralloc(char const *s){
	size_t len;
	char *x;


	len = strlen(s);
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
