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
	#include <stdarg.h>
	#include <stdio.h>
	#include <string.h>
	#include <sys/escape.h>
	#include <lexer.lex.h>
	#include "assert.h"
	#include "node.h"
	#include "type.h"


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
		\
		_s; \
	})

	#define EXPR_ALLOC(e)({ \
		expr_t *_e = expr_alloc(e); \
		EABORT(_e == 0x0); \
		\
		_e; \
	})

	#define EXPR_COPY(dest, src) \
		EABORT(expr_copy(dest, src) != 0);

	#define EXPR_ARRAY_ADD(array, el) \
		EABORT(expr_array_add(array, el) != 0);

	#define ATTR_INIT(attr, name, type, array_limit, value)({ \
		attr_t *_ini = attr; \
		EABORT(attr_init(_ini, name, type, array_limit, value) == 0x0); \
		\
		_ini; \
	})

	#define ATTRVEC_ADD(attrs, attr) \
		EABORT(attrvec_add(attrs, attr) != 0)

	#define ATTR_ASSIGN(attr, value)({ \
		attr_t *_assi = attr; \
		EABORT(attr_assign(_assi, value) == 0x0) \
		\
		_assi; \
	})

	#define ATTR_REF(node_name, attr_name)({ \
		node_t *_nref = node_ref(node_name); \
		EABORT(_nref == 0x0); \
		\
		attr_t *_aref = attrvec_query(&_nref->attrs, attr_name, false); \
		EABORT(_aref == 0x0); \
		\
		_aref; \
	})

	#define TYPE_CREATE(name, attrs, asserts) \
		EABORT(type_create(name, attrs, asserts) != 0);

	#define TYPE_RESET(type){ \
		(type).attrs = ATTRVEC_INITIALISER(); \
		(type).asserts = 0x0; \
	}

	#define TYPE_LOOKUP(name)({ \
		type_t *_t = type_lookup(name); \
		EABORT(_t == 0x0); \
		\
		_t; \
	})

	#define TYPE_INSTANTIATE(type_name, node_name, node)({ \
		node_t *_n = node; \
		node_t *_r = type_instantiate(TYPE_LOOKUP(type_name), node_name, &_n->attrs, _n->childs); \
		EABORT(_r == 0x0); \
		\
		_r; \
	})

	#define NODE_RESET(node){ \
		(node).attrs = ATTRVEC_INITIALISER(); \
		(node).childs = 0x0; \
	}

	#define EXPR_INIT(expr, op, arg0, arg1)({ \
		EABORT(expr_init(expr, op, arg0, arg1) == 0x0); \
		expr; \
	})

	#define ASSERT_CREATE(expr, msg)({ \
		assert_t *_a; \
		_a = assert_create(expr, msg); \
		EABORT(_a == 0x0); \
		\
		_a; \
	})


	/* local/static variables */
	static FILE *fp = 0;
	static char const *dt_script = 0x0;
	static bool erroneous = false;


	/* prototypes */
	void devtreeunput(char c);


	/* local/static prototypes */
static int devtreeerror(char const *file, char const *s);
	static int cleanup(void);
	static expr_t *expr_init(expr_t *expr, expr_op_t op, expr_t *arg0, expr_t *arg1);
	static void *stralloc(char const *s);
%}

%code requires{
	#include <sys/list.h>
	#include <sys/vector.h>
	#include "assert.h"
	#include "attr.h"
	#include "attrvec.h"
	#include "expr.h"
	#include "node.h"
	#include "type.h"


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

	if(fp == 0x0){
		fprintf(stderr, "read device tree script \"%s\" failed \"%s\"\n", file, strerror(errno));
		return 1;
	}

	/* start lexer */
	devtreerestart(fp);
}

/* parser union type */
%union{
	char *str_p;
	attr_t *attr_p;
	node_t *node_p;
	assert_t *assert_p;

	type_t type;
	node_t node;
	attr_t type_attr;
	expr_t expr;
	expr_type_t builtin_type;
	EXPR_INT_T i;
	char s[DEVTREE_STRMAX];
}

/* terminals */
// general
%token <i> INT
%token <s> STRING
%token <s> IDFR

// typedef
%token TYPEDEF

// node attributes
%token NA_ADDR
%token NA_INT8
%token NA_INT16
%token NA_INT32
%token NA_INT64
%token NA_STRING

// operators
%token ASSIGN
%token EQUAL
%token UNEQUAL
%token LESSER
%token GREATER
%token LEFTSHIFT
%token RIGHTSHIFT
%token PLUS
%token MINUS
%token MULTIPLY
%token DIVIDE
%token MODULO
%token LOGAND
%token LOGOR
%token BITAND
%token BITOR
%token BITXOR
%token LESSEREQ
%token GREATEREQ
%token LSHIFTEQ
%token RSHIFTEQ
%token PLUSEQ
%token MINUSEQ
%token MULTIPLYEQ
%token DIVIDEEQ
%token MODULOEQ
%token UNARYPLUS
%token UNARYMINUS

// asserts
%token ASSERT

/* non-terminals */
%type <type> type-body
%type <type_attr> type-attr
%type <assert_p> assert

%type <node_p> node
%type <node> node-args

%type <builtin_type> builtin-type

%type <expr> expression
%type <expr> logor
%type <expr> logand
%type <expr> bitor
%type <expr> bitxor
%type <expr> bitand
%type <expr> equality
%type <expr> relational
%type <expr> shift
%type <expr> additive
%type <expr> multiplicative
%type <expr> unary
%type <expr> value

%type <attr_p> attr-ref
%type <expr> array
%type <expr> array-body
%type <expr> const
%type <str_p> string


%%


/* start */
start : devtree											{ if(cleanup() != 0) YYABORT; };

/* sections */
devtree : %empty										{ }
		| error ';'										{ erroneous = true; yyerrok; }
		| devtree ';'									{ }
		| devtree typedef ';'							{ }
		| devtree node ';'								{ list_add_tail(nodes_root()->childs, $2); }
		;

/* typedef */
typedef : TYPEDEF '{' type-body '}' IDFR				{ TYPE_CREATE(STRALLOC($5), &$3.attrs, $3.asserts); }
		;

type-body : %empty										{ TYPE_RESET($$); }
		  | type-body ';'								{ $$ = $1; }
		  | type-body assert ';'						{ $$ = $1; list_add_tail($$.asserts, $2); }
		  | type-body type-attr ';'						{ $$ = $1; ATTRVEC_ADD(&$$.attrs, &$2); }
		  | type-body type-attr ASSIGN expression ';'	{ $$ = $1; ATTRVEC_ADD(&$$.attrs, ATTR_ASSIGN(&$2, &$4)); }
		  ;

type-attr : builtin-type IDFR							{ ATTR_INIT(&$$, STRALLOC($2), $1, 0, 0x0); }
		  | builtin-type IDFR '[' INT ']'				{ ATTR_INIT(&$$, STRALLOC($2), $1, $4, 0x0); }
		  ;

builtin-type : NA_STRING								{ $$ = ET_STRING; }
			 | NA_ADDR									{ $$ = ET_ADDR; }
			 | NA_INT8   								{ $$ = ET_INT8; }
			 | NA_INT16									{ $$ = ET_INT16; }
			 | NA_INT32									{ $$ = ET_INT32; }
			 | NA_INT64									{ $$ = ET_INT64; }
			 ;

assert : ASSERT '(' expression ',' string ')'			{ $$ = ASSERT_CREATE(EXPR_ALLOC(&$3), $5); };

/* nodes */
node : IDFR ASSIGN IDFR '(' node-args ')'				{ $$ = TYPE_INSTANTIATE($3, STRALLOC($1), &$5); }
	 | IDFR ASSIGN IDFR '(' node-args ',' ')'			{ $$ = TYPE_INSTANTIATE($3, STRALLOC($1), &$5); }
	 ;

node-args : %empty										{ NODE_RESET($$); devtreeunput(','); }
		  | node-args ',' node							{ $$ = $1; list_add_tail($$.childs, $3); }
		  | node-args ',' IDFR ASSIGN expression		{ $$ = $1; ATTRVEC_ADD(&$$.attrs, ATTR_ASSIGN(ATTR_INIT(&(attr_t){ }, STRALLOC($3), expr_type(&$5), 0, 0x0), &$5)); }
		  ;

/* expression */
expression : logor										{ $$ = $1; };

logor : logand											{ $$ = $1; }
	  | logor LOGOR logand								{ EXPR_INIT(&$$, EOP_LOG_OR, &$1, &$3); }
	  ;

logand : bitor											{ $$ = $1; }
	   | logand LOGAND bitor							{ EXPR_INIT(&$$, EOP_LOG_AND, &$1, &$3); }
	   ;

bitor : bitxor											{ $$ = $1; }
	  | bitor BITOR bitxor								{ EXPR_INIT(&$$, EOP_BIT_OR, &$1, &$3); }

bitxor : bitand											{ $$ = $1; }
	   | bitxor BITXOR bitand							{ EXPR_INIT(&$$, EOP_BIT_XOR, &$1, &$3); }
	   ;

bitand : equality										{ $$ = $1; }
	   | bitand BITAND equality							{ EXPR_INIT(&$$, EOP_BIT_AND, &$1, &$3); }
	   ;

equality : relational									{ $$ = $1; }
		 | equality EQUAL relational					{ EXPR_INIT(&$$, EOP_EQUAL, &$1, &$3); }
		 | equality UNEQUAL relational					{ EXPR_INIT(&$$, EOP_UNEQUAL, &$1, &$3); }
		 ;

relational : shift										{ $$ = $1; }
		   | relational LESSER shift					{ EXPR_INIT(&$$, EOP_LESSER, &$1, &$3); }
		   | relational GREATER shift					{ EXPR_INIT(&$$, EOP_LESSER_EQUAL, &$1, &$3); }
		   | relational LESSEREQ shift					{ EXPR_INIT(&$$, EOP_GREATER, &$1, &$3); }
		   | relational GREATEREQ shift					{ EXPR_INIT(&$$, EOP_GREATER_EQUAL, &$1, &$3); }
		   ;

shift : additive										{ $$ = $1; }
	  | shift LEFTSHIFT additive						{ EXPR_INIT(&$$, EOP_LEFT_SHIFT, &$1, &$3); }
	  | shift RIGHTSHIFT additive						{ EXPR_INIT(&$$, EOP_RIGHT_SHIFT, &$1, &$3); }
	  ;

additive : multiplicative								{ $$ = $1; }
		 | additive PLUS multiplicative					{ EXPR_INIT(&$$, EOP_ADD, &$1, &$3); }
		 | additive MINUS multiplicative				{ EXPR_INIT(&$$, EOP_SUBTRACT, &$1, &$3); }
		 ;

multiplicative : unary									{ $$ = $1; }
			   | multiplicative MULTIPLY unary			{ EXPR_INIT(&$$, EOP_MULTIPLY, &$1, &$3); }
			   | multiplicative DIVIDE unary			{ EXPR_INIT(&$$, EOP_DIVIDE, &$1, &$3); }
			   | multiplicative MODULO unary			{ EXPR_INIT(&$$, EOP_MODULO, &$1, &$3); }
			   ;

// TODO check for proper memory clearance
unary : attr-ref UNARYPLUS								{ $$ = *$1->value; ATTR_ASSIGN($1, EXPR_INIT($1->value, EOP_ADD, $1->value, &EXPR_INT(64, 1))); }
	  | attr-ref UNARYMINUS								{ $$ = *$1->value; ATTR_ASSIGN($1, EXPR_INIT($1->value, EOP_SUBTRACT, $1->value, &EXPR_INT(64, 1))); }
	  | UNARYPLUS attr-ref								{ ATTR_ASSIGN($2, EXPR_INIT($2->value, EOP_ADD, $2->value, &EXPR_INT(64, 1))); $$ = *$2->value; }
	  | UNARYMINUS attr-ref								{ ATTR_ASSIGN($2, EXPR_INIT($2->value, EOP_SUBTRACT, $2->value, &EXPR_INT(64, 1))); $$ = *$2->value; }
	  | value											{ $$ = $1; }
	  ;

value : const											{ $$ = $1; }
	  | array											{ $$ = $1; }
	  | attr-ref										{ EXPR_COPY(&$$, $1->value); }
	  | '(' expression ')'								{ $$ = $2; }
	  | IDFR											{ $$ = EXPR_REF(STRALLOC($1)); }
	  ;

/* values */
attr-ref : IDFR '.' IDFR								{ $$ = ATTR_REF($1, $3); };

array : '[' array-body ']'								{ $$ = $2; }
	  | '[' array-body ',' ']'							{ $$ = $2; }
	  ;

array-body : %empty										{ $$ = EXPR_ARRAY(ET_UNDEF); devtreeunput(','); }
		   | array-body ',' expression					{ $$ = $1; EXPR_ARRAY_ADD(&$$, &$3); }
		   ;

const : INT												{ $$ = EXPR_INT(64, $1); }
	  | string											{ $$ = EXPR_STR($1); }
	  ;

string : STRING											{ $$ = STRALLOC($1); };


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

static expr_t *expr_init(expr_t *expr, expr_op_t op, expr_t *arg0, expr_t *arg1){
	expr_value_t r;


	*expr = EXPR(op, EXPR_VALUE_EXPR(arg0), EXPR_VALUE_EXPR(arg1));
	expr_print(expr, 1);
	printf("\n");

	if(expr_evaluate(expr, &r, 0x0) == 0x0){
		printf("  unable to reduce");
		arg0 = expr_alloc(arg0);
		arg1 = expr_alloc(arg1);

		if(arg0 == 0x0 || arg1 == 0x0)
			goto err;

		*expr = EXPR(op, EXPR_VALUE_EXPR(arg0), EXPR_VALUE_EXPR(arg0));
	}
	else{
		*expr = EXPR_LITERAL(r);
		printf("  reduced to");
		expr_print(expr, 3);
	}

	printf("\n");

	return expr;


err:
	expr_free(arg0);
	expr_free(arg1);

	return 0x0;
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
