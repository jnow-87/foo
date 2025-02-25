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
		if(expr) \
			YYERROR; \
	}

	// helper
	#define STRALLOC(s)({ \
		void *_s = stralloc(s); \
		EABORT(_s == 0x0); \
		_s; \
	})

	#define ATTR_INIT(attr, name, type, array_limit, value)({ \
		attr_t *_ini = attr; \
		EABORT(attr_init(_ini, name, type, array_limit, value) != _ini); \
		_ini; \
	})

	#define UNNAMED_ATTR(type, value)({ \
		attr_t *_new = ATTR_UNNAMED_##type(value); \
		EABORT(_new == 0x0); \
		\
		_new; \
	})

	#define UNNAMED_INT(value)		UNNAMED_ATTR(INT, value)
	#define UNNAMED_STRING(value)	UNNAMED_ATTR(STRING, value)

	#define ATTR_ENLIST(attrs, attr) \
		EABORT(attr_enlist(attrs, attr) != 0)

	#define ATTR_ASSIGN(attr, value)({ \
		attr_t *_assi = attr; \
		EABORT(attr_assign(_assi, value) != _assi) \
		_assi; \
	})

	#define ATTR_ADD(value, op) \
		EABORT(attr_add(value, op)); \

	#define ATTR_COPY(dest, src) \
		EABORT(attr_copy(dest, src))

	#define ATTR_REF(node_name, attr_name)({ \
		node_t *_nref = node_ref(node_name); \
		EABORT(_nref == 0x0); \
		\
		attr_t *_aref = attr_get(&_nref->attrs, attr_name, false); \
		EABORT(_aref == 0x0); \
		\
		_aref; \
	})

	#define TYPE_LOOKUP(name)({ \
		type_t *_type = type_lookup(name); \
		EABORT(_type == 0x0); \
		\
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
	static bool erroneous = false;


	/* prototypes */
	void devtreeunput(char c);


	/* local/static prototypes */
	static int devtreeerror(char const *file, char const *s);
	static int cleanup(void);
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
		fprintf(stderr, "read device tree script \"%s\" failed \"%s\"\n", file, strerror(errno));
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
	attr_t attr;

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
%type <attr> type-attr
%type <assert> assert

%type <node> node
%type <object> node-args

%type <aptr> attr-ref
%type <attr> attr-inc

%type <attr> value
%type <attr> array
%type <attr> array-body
%type <attr> const
%type <sptr> string

%type <type> type


%%


/* start */
start : devtree										{ if(cleanup() != 0) YYABORT; };

/* sections */
devtree : %empty									{ }
		| error ';'									{ erroneous = true; yyerrok; }
		| devtree ';'								{ }
		| devtree typedef ';'						{ }
		| devtree node ';'							{ EABORT(node_child_add(nodes_root($2->type->category), $2)); }
		| devtree attr-op ';'						{ }
		;

/* typedef */
typedef : TYPEDEF_MEM '{' type-body '}' IDFR		{ EABORT(type_add(STRALLOC($5), TC_MEMORY, &$3.attrs, $3.asserts)); }
		| TYPEDEF_DEV '{' type-body '}' IDFR		{ EABORT(type_add(STRALLOC($5), TC_DEVICE, &$3.attrs, $3.asserts)); }
		;

type-body : %empty									{ OBJECT_RESET($$); }
		  | type-body ';'							{ }
		  | type-body assert ';'					{ $$ = $1; list_add_tail($$.asserts, $2); }
		  | type-body type-attr ';'					{ $$ = $1; ATTR_ENLIST(&$$.attrs, &$2); }
		  | type-body type-attr '=' const ';'		{ $$ = $1; ATTR_ENLIST(&$$.attrs, ATTR_ASSIGN(&$2, &$4)); }
		  | type-body type-attr '=' array ';'		{ $$ = $1; ATTR_ENLIST(&$$.attrs, ATTR_ASSIGN(&$2, &$4)); }
		  ;

type-attr : type IDFR								{ ATTR_INIT(&$$, STRALLOC($2), $1, 0, 0x0); }
		  | type IDFR '[' INT ']'					{ ATTR_INIT(&$$, STRALLOC($2), $1, $4, 0x0);}

assert : ASSERT '(' string ',' string ')'			{ $$ = assert_create($3, $5); EABORT($$ == 0x0); };

/* nodes */
node : IDFR '=' IDFR '(' node-args ')'				{ $$ = type_instantiate(TYPE_LOOKUP($3), STRALLOC($1), &$5.attrs, $5.childs); EABORT($$ == 0x0); }
	 | IDFR '=' IDFR '(' node-args ',' ')'			{ $$ = type_instantiate(TYPE_LOOKUP($3), STRALLOC($1), &$5.attrs, $5.childs); EABORT($$ == 0x0); }
	 ;

node-args : %empty									{ OBJECT_RESET($$); devtreeunput(','); }
		  | node-args ',' node						{ $$ = $1; list_add_tail($$.childs, $3); }
		  | node-args ',' IDFR '=' value			{ $$ = $1; ATTR_ENLIST(&$$.attrs, ATTR_ASSIGN(ATTR_INIT(&(attr_t){}, STRALLOC($3), AT_UNDEF, 0, 0x0), &$5)); }
		  ;

/* attributes */
attr-ref : IDFR '.' IDFR							{ $$ = ATTR_REF($1, $3); };

attr-op : attr-ref '=' value						{ ATTR_ASSIGN($1, &$3); }
		| attr-ref '+' '=' value					{ ATTR_ADD($1, &$4); }
		| attr-inc									{ }
		;

attr-inc : attr-ref '+' '+'							{ ATTR_COPY(&$$, $1); ATTR_ADD($1, UNNAMED_ATTR(INT, 1)); }
		 | '+' '+' attr-ref							{ ATTR_ADD($3, UNNAMED_ATTR(INT, 1)); ATTR_COPY(&$$, $3); }
		 ;

/* attribute values */
value : const										{ $$ = $1; }
	  | array										{ $$ = $1; }
	  | attr-ref									{ ATTR_COPY(&$$, $1); }
	  | value '+' const								{ $$ = $1; ATTR_ADD(&$$, &$3); }
	  | value '+' array								{ $$ = $1; ATTR_ADD(&$$, &$3); }
	  | value '+' attr-ref							{ $$ = $1; ATTR_ADD(&$$, $3); }
	  | '(' attr-inc ')'							{ $$ = $2; }
	  ;

array : '[' array-body ']'							{ $$ = $2; }
	  | '[' array-body ',' ']'						{ $$ = $2; }
	  ;

array-body : %empty									{ ATTR_INIT(&$$, 0x0, AT_UNDEF, ATTR_ARRAY_UNLIMITED, 0x0); devtreeunput(','); }
		   | array-body ',' value					{ $$ = $1; ATTR_ADD(&$$, ($3.flags & AF_ARRAY) ? &$3 : attr_convert_to_list(&$3)); }
		   ;

const : INT											{ $$ = *UNNAMED_INT($1); }
	  | string										{ $$ = *UNNAMED_STRING($1); }
	  ;

string : STRING										{ $$ = STRALLOC($1); };


/* node attributes */
type : NA_STRING									{ $$ = AT_STRING; }
	 | NA_ADDR										{ $$ = AT_ADDR; }
	 | NA_INT8   									{ $$ = AT_INT8; }
	 | NA_INT16										{ $$ = AT_INT16; }
	 | NA_INT32										{ $$ = AT_INT32; }
	 | NA_INT64										{ $$ = AT_INT64; }
	 ;


%%


/* global functions */
int devtree_parser_error(char const *fmt, ...){
	va_list lst;


	if(dt_script != 0x0){
		fprintf(stderr, FG("%s", PURPLE) ":" FG("%d:%d", GREEN) " token \"%s\" -- ",
			dt_script,
			devtreelloc.first_line,
			devtreelloc.first_column,
			devtreetext
		);
	}

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

static int cleanup(void){
	bool r = erroneous;


	devtreelex_destroy();
	fclose(fp);

	dt_script = 0x0;
	erroneous = false;

	return r ? -1 : 0;
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
