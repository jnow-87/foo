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

	#define ATTR_ASSIGN(attrs, name, value) \
		EABORT(attr_assign(attrs, name, value) != 0)

	#define ATTR_INIT(attr, value) \
		EABORT(attr_init(attr, 0x0, value));

	#define ATTR_ADD(value, op) \
		EABORT(attr_add(value, op)); \

	#define ATTR_COPY(dest, src) \
		EABORT(attr_copy(dest, src))

	#define ATTR_ILIST_ADD(lst, value) \
		EABORT(attr_ilist_add(lst, value))

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

	#define ATTR_TYPE_CHECK(value, type) \
		EABORT(attr_type_check(value, type) != 0)

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
	ATTR_INT_TYPE i;
	char s[DEVTREE_STRMAX];
	char *sptr;
	attr_t *aptr;
	attr_type_t type;
	attr_value_t value;

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
%token <s> STRING
%token <s> IDFR

// typedef
%token TYPEDEF_MEM
%token TYPEDEF_DEV

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
%type <type> xattr
%type <value> ilist
%type <value> opt-int
%type <sptr> string
%type <aptr> attr-ref
%type <value> attr-inc
%type <value> value
%type <value> const


%%


/* start */
start : error										{ cleanup(); YYABORT; }
	  | section-lst									{ cleanup(); }
	  ;

/* sections */
section-lst : %empty								{ }
			| section-lst ';'						{ }
			| section-lst typedef ';'				{ }
			| section-lst device ';'				{ CHILD_ADD(nodes_root($2->type->category), $2); }
			| section-lst attr-update ';'			{ }
			;

/* typedef */
typedef : TYPEDEF_MEM '{' type-body '}' IDFR		{ EABORT(type_add(STRALLOC($5), TC_MEMORY, &$3.attrs, $3.asserts)); }
		| TYPEDEF_DEV '{' type-body '}' IDFR		{ EABORT(type_add(STRALLOC($5), TC_DEVICE, &$3.attrs, $3.asserts)); }
		;

type-body : %empty									{ OBJECT_RESET($$); }
		  | type-body ';'							{ }
		  | type-body assert ';'					{ $$ = $1; list_add_tail($$.asserts, $2); }
		  | type-body xattr IDFR ';'				{ $$ = $1; ATTR_ASSIGN(&$$.attrs, STRALLOC($3), &ATTR_NOVALUE($2)); }
		  | type-body xattr IDFR '=' const ';'		{ $$ = $1; ATTR_TYPE_CHECK(&$5, $2); $5.type = $2; ATTR_ASSIGN(&$$.attrs, STRALLOC($3), &$5); }
		  | type-body xattr IDFR '[' INT ']' ';'	{ $$ = $1; ATTR_TYPE_CHECK(&ATTR_NOVALUE($2), MT_INT64); ATTR_ASSIGN(&$$.attrs, STRALLOC($3), &ATTR_VALUE_ILIST((ATTR_ILIST($5)), $2)); /* TODO list size is ignored by now */ }
		  | type-body xattr IDFR '[' INT ']' '=' ilist ';'	{ $$ = $1; EABORT($5 != $8.ilist.limit); /* TODO error message */ ATTR_ASSIGN(&$$.attrs, STRALLOC($3), &$8); }
		  ;

/* nodes */
device : IDFR '=' IDFR '(' type-args ')'			{ $$ = type_instantiate(TYPE_LOOKUP($3), STRALLOC($1), &$5.attrs, $5.childs); EABORT($$ == 0x0); }
	   | IDFR '=' IDFR '(' type-args ',' ')'		{ $$ = type_instantiate(TYPE_LOOKUP($3), STRALLOC($1), &$5.attrs, $5.childs); EABORT($$ == 0x0); }
	   ;

type-args : %empty									{ OBJECT_RESET($$); devtreeunput(','); }
		  | type-args ',' device					{ $$ = $1; list_add_tail($$.childs, $3); }
		  | type-args ',' IDFR '=' value			{ $$ = $1; ATTR_ASSIGN(&$$.attrs, STRALLOC($3), &$5); }
		  ;

/* asserts */
assert : ASSERT '(' string ',' string ')'			{ $$ = CREATE(assert, $3, $5); };

/* attribute updates */
attr-update : attr-ref '=' value					{ ATTR_TYPE_CHECK(&$1->value, $3.type); $1->value = $3; }
			| attr-ref '+' '=' value				{ ATTR_ADD(&$1->value, &$4); }
			| attr-inc								{ }
			;

attr-inc : attr-ref '+' '+'							{ ATTR_TYPE_CHECK(&$1->value, MT_INT64); $$ = ATTR_VALUE_INT(($1->value.i)++, $1->value.type); }
		 | '+' '+' attr-ref							{ ATTR_TYPE_CHECK(&$3->value, MT_INT64); $$ = ATTR_VALUE_INT(++($3->value.i), $3->value.type); }
		 ;

/* basic types */
value : const										{ $$ = $1; }
	  | ilist										{ $$ = $1; }
	  | attr-ref									{ ATTR_COPY(&$$, &$1->value); }
	  | value '+' const								{ $$ = $1; ATTR_ADD(&$$, &$3); }
	  | value '+' ilist								{ $$ = $1; ATTR_ADD(&$$, &$3); }
	  | value '+' attr-ref							{ $$ = $1; ATTR_ADD(&$$, &$3->value); }
	  | '(' attr-inc ')'							{ $$ = $2; }
	  ;

ilist : '[' opt-int ']'								{ $$ = $2; }
	  | '[' opt-int ',' ']'							{ $$ = $2; }
	  ;

opt-int : %empty									{ $$ = ATTR_VALUE_ILIST((ATTR_ILIST(0)), MT_INT64); devtreeunput(','); }
		| opt-int ',' INT							{ $$ = $1; ATTR_ILIST_ADD(&$$.ilist, $3); }
		| opt-int ',' attr-ref						{ $$ = $1; ATTR_TYPE_CHECK(&$3->value, MT_INT64); ATTR_ILIST_ADD(&$$.ilist, $3->value.i); }
		;

attr-ref : IDFR '.' IDFR							{ $$ = ATTR_REF(&NODE_REF($1)->attrs, $3); };
string : STRING										{ $$ = STRALLOC($1); };

const : INT											{ $$ = ATTR_VALUE_INT($1, MT_INT64); }
	  | string										{ $$ = ATTR_VALUE_STRING($1); }
	  ;

/* node attributes */
xattr : NA_STRING									{ $$ = MT_STRING; }
	  | NA_ADDR										{ $$ = MT_ADDR; }
	  | NA_INT8   									{ $$ = MT_INT8; }
	  | NA_INT16									{ $$ = MT_INT16; }
	  | NA_INT32									{ $$ = MT_INT32; }
	  | NA_INT64									{ $$ = MT_INT64; }
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
