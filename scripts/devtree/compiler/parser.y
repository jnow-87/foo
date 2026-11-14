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
	#include "array.h"
	#include "assert.h"
	#include "node.h"
	#include "strmap.h"
	#include "type.h"


	/* macros */
	#define YYDEBUG	1

	// parser abort on error
	#define EABORT(expr){ \
		if(expr) \
			YYERROR; \
	}

	#define EABORT_VAL(op, type, cmp, tmp_name)({ \
		type tmp_name = op; \
		EABORT(tmp_name cmp); \
		tmp_name; \
	})

	// helper
	#define TYPE_ALLOC(name, attrs, asserts)		EABORT(type_alloc_clean(name, attrs, asserts) != 0)
	#define TYPE_LOOKUP(name)						EABORT_VAL(type_lookup(name), type_t *, == 0x0, _type)
	#define TYPE_ATTR_ADD(type, attr, value)		EABORT(type_attr_add_clean(type, attr, value) != 0)

	#define TYPE_ATTR_INIT(attr, name, type, is_array, array_limit) \
		EABORT(type_attr_init(attr, name, type, is_array, array_limit) != 0)

	#define TYPE_INSTANTIATE(type_name, node_name, attrs, childs) \
		EABORT_VAL(type_instantiate_clean(type_name, node_name, attrs, childs), node_t *, == 0x0, _inst)

	#define NODE_ATTR_ADD(node, name, expr)			EABORT(node_attr_add(node, name, expr) != 0)
	#define NODE_ATTR_UPDATE(attr_ref, op, expr)	EABORT(node_attr_update(attr_ref.node_p, attr_ref.attr_p, op, expr) != 0)
	#define NODE_REF(name)							EABORT_VAL(node_ref(name), node_t *, == 0x0, _nref)

	#define NODE_ATTR_INIT(attr, name, value) \
		EABORT_VAL(node_attr_init(attr, name, value), node_attr_t *, == 0x0, _nini)

	#define ASSERT_ALLOC(expr, msg)					EABORT_VAL(assert_alloc(expr, msg), assert_t *, == 0x0, _ass)

	#define EXPR_INIT(expr, op, arg0, arg1)			EABORT_VAL(expr_init(expr, op, arg0, arg1), expr_t *, == 0x0, _eini)
	#define EXPR_ALLOC(e)							EABORT_VAL(expr_alloc(e), expr_t *, == 0x0, _expr)
	#define EXPR_ARRAY_ADD(arr, el)					EABORT(expr_array_add_clean(arr, el) != 0);

	#define ATTR_REF(node_ref, attr_name)			EABORT_VAL(attrvec_query(&node_ref->attrs.v, attr_name, false), attr_t *, == 0x0, _aref)

	#define STR_ALLOC(s)							EABORT_VAL(str_cache_add(s), char const *, == 0x0, _s)
	#define ARRAY_ALLOC()							EABORT_VAL(array_alloc(), array_t *, == 0x0, _a)
	#define ARRAY_CACHE(arr)						EABORT_VAL(array_cache_add(arr), array_t *, == 0x0, _a)


	/* static variables */
	static FILE *fp = 0;
	static char const *dt_script = 0x0;
	static bool erroneous = false;


	/* prototypes */
	void devtreeunput(char c);


	/* local/static prototypes */
	static int devtreeerror(char const *file, char const *s);
	static int cleanup(void);

	static int type_alloc_clean(char const *name, type_attrvec_t *attrs, assert_t *asserts);
	static int type_reset(type_t *type, bool clean);
	static int type_attr_add_clean(type_t *type, type_attr_t *attr, expr_t *value);
	static node_t *type_instantiate_clean(char const *type_name, char const *node_name, node_attrvec_t *attrs, node_t *childs);

	static int node_attr_add(node_t *node, char const *name, expr_t *value);
	static int node_reset(node_t *node, bool clean);

	static expr_t *expr_init(expr_t *expr, expr_op_t op, expr_t *arg0, expr_t *arg1);
	static int expr_array_add_clean(expr_t *arr, expr_t *expr);

	static int attr_reset(type_attr_t *attr, bool clean);

%}

%code requires{
	#include <sys/list.h>
	#include <sys/vector.h>
	#include "assert.h"
	#include "attr.h"
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
	char const *str_p;
	node_t *node_p;
	assert_t *assert_p;

	struct{
		node_attr_t *attr_p;
		node_t *node_p;
	} attr_ref;

	type_t type;
	node_t node;
	type_attr_t attr;
	expr_t expr;
	expr_type_t builtin_type;
	EXPR_INT_T i;
	char s[DEVTREE_STRMAX];
}

%destructor { type_reset(&$$, true); } <type>
%destructor { node_reset(&$$, true); } <node>
%destructor { attr_reset(&$$, true); } <attr>
%destructor { expr_reset(&$$); } <expr>

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

// asserts
%token ASSERT

/* non-terminals */
%type <type> type-body
%type <attr> type-attr
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

%type <attr_ref> attr-ref
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
typedef : TYPEDEF '{' type-body '}' IDFR				{ TYPE_ALLOC(STR_ALLOC($5), &$3.attrs, $3.asserts); }
		;

type-body : %empty										{ type_reset(&$$, false); }
		  | type-body ';'								{ $$ = $1; }
		  | type-body assert ';'						{ $$ = $1; list_add_tail($$.asserts, $2); }
		  | type-body type-attr ';'						{ $$ = $1; TYPE_ATTR_ADD(&$$, &$2, 0x0); }
		  | type-body type-attr ASSIGN expression ';'	{ $$ = $1; TYPE_ATTR_ADD(&$$, &$2, EXPR_ALLOC(&$4)); }
		  ;

type-attr : builtin-type IDFR							{ TYPE_ATTR_INIT(&$$, STR_ALLOC($2), $1, false, 0); }
		  | builtin-type IDFR '[' INT ']'				{ TYPE_ATTR_INIT(&$$, STR_ALLOC($2), $1, true, $4); }
		  ;

builtin-type : NA_STRING								{ $$ = ET_STRING; }
			 | NA_ADDR									{ $$ = ET_ADDR; }
			 | NA_INT8   								{ $$ = ET_INT8; }
			 | NA_INT16									{ $$ = ET_INT16; }
			 | NA_INT32									{ $$ = ET_INT32; }
			 | NA_INT64									{ $$ = ET_INT64; }
			 ;

assert : ASSERT '(' expression ',' string ')'			{ $$ = ASSERT_ALLOC(EXPR_ALLOC(&$3), $5); };

/* nodes */
node : IDFR ASSIGN IDFR '(' node-args ')'				{ $$ = TYPE_INSTANTIATE($3, STR_ALLOC($1), &$5.attrs, $5.childs); }
	 | IDFR ASSIGN IDFR '(' node-args ',' ')'			{ $$ = TYPE_INSTANTIATE($3, STR_ALLOC($1), &$5.attrs, $5.childs); }
	 ;

node-args : %empty										{ node_reset(&$$, false); devtreeunput(','); }
		  | node-args ',' node							{ $$ = $1; list_add_tail($$.childs, $3); }
		  | node-args ',' IDFR ASSIGN expression		{ $$ = $1; NODE_ATTR_ADD(&$$, STR_ALLOC($3), &$5); }
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

unary : attr-ref UNARYPLUS								{ $$ = EXPR_LITERAL($1.attr_p->value); NODE_ATTR_UPDATE($1, EOP_ADD, &EXPR_INT(64, 1)); }
	  | UNARYPLUS attr-ref								{ NODE_ATTR_UPDATE($2, EOP_ADD, &EXPR_INT(64, 1)); $$ = EXPR_LITERAL($2.attr_p->value); }
	  | value											{ $$ = $1; }
	  ;

value : const											{ $$ = $1; }
	  | array											{ $$ = EXPR_ARRAY($$.literal.type, ARRAY_CACHE(&($1.literal.array))); }
	  | attr-ref										{ $$ = EXPR_LITERAL($1.attr_p->value); }
	  | '(' expression ')'								{ $$ = $2; }
	  | IDFR											{ $$ = EXPR_REF(STR_ALLOC($1)); }
	  ;

/* values */
attr-ref : IDFR '.' IDFR								{ $$.node_p = NODE_REF($1); $$.attr_p = (node_attr_t*)ATTR_REF($$.node_p, $3); };

array : '[' array-body ']'								{ $$ = $2; }
	  | '[' array-body ',' ']'							{ $$ = $2; }
	  ;

array-body : %empty										{ $$ = EXPR_ARRAY(ET_UNDEF, ARRAY_ALLOC()); devtreeunput(','); }
		   | array-body ',' expression					{ $$ = $1; EXPR_ARRAY_ADD(&$$, &$3); }
		   ;

const : INT												{ $$ = EXPR_INT(64, $1); }
	  | string											{ $$ = EXPR_STR($1); }
	  ;

string : STRING											{ $$ = STR_ALLOC($1); };


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

static int type_alloc_clean(char const *name, type_attrvec_t *attrs, assert_t *asserts){
	if(type_alloc(name, attrs, asserts) != 0)
		return type_reset(&(type_t){ .attrs = *attrs, .asserts = asserts }, true);

	return 0;
}

static int type_reset(type_t *type, bool clean){
	if(clean){
		asserts_free(type->asserts);
		type_attrvec_reset(&type->attrs);
	}

	type->attrs = TYPE_ATTRVEC_INITIALISER();
	type->asserts = 0x0;

	return -1;
}

static int type_attr_add_clean(type_t *type, type_attr_t *attr, expr_t *value){
	if(value != 0x0 && type_attr_assign(attr, value) == 0x0)
		goto err;

	if(attrvec_add(&type->attrs.v, (attr_t*)attr, sizeof(type_attr_t)) != 0)
		goto err;

	return 0;


err:
	if(value)
		expr_free(value);

	asserts_free(type->asserts);
	type_attrvec_reset(&type->attrs);

	return -1;
}

static node_t *type_instantiate_clean(char const *type_name, char const *node_name, node_attrvec_t *attrs, node_t *childs){
	node_t *n = 0x0;
	type_t *t;


	t = type_lookup(type_name);

	if(t != 0x0)
		n = type_instantiate(t, node_name, attrs, childs);

	node_attrvec_reset(attrs);

	return n;
}

static int node_attr_add(node_t *node, char const *name, expr_t *value){
	expr_value_t v;
	node_attr_t attr;


	if(expr_evaluate(value, &v, 0x0) == 0x0)
		goto err;

	if(node_attr_init(&attr, name, &v) == 0x0)
		goto err;

	if(attrvec_add(&node->attrs.v, (attr_t*)&attr, sizeof(node_attr_t)) != 0)
		goto err;

	return 0;


err:
	node_attrvec_reset(&node->attrs);

	return -1;
}

static int node_reset(node_t *node, bool clean){
	if(clean)
		node_attrvec_reset(&node->attrs);

	node->attrs = NODE_ATTRVEC_INITIALISER();
	node->childs = 0x0;

	return -1;
}

static expr_t *expr_init(expr_t *expr, expr_op_t op, expr_t *arg0, expr_t *arg1){
	expr_value_t r;


	*expr = EXPR_CALC(op, arg0, arg1);

	if(expr_evaluate(expr, &r, 0x0) == 0x0){
		arg0 = expr_alloc(arg0);
		arg1 = expr_alloc(arg1);

		if(arg0 == 0x0 || arg1 == 0x0)
			goto err;

		*expr = EXPR_CALC(op, arg0, arg1);
	}
	else
		*expr = EXPR_LITERAL(r);

	return expr;


err:
	expr_free(arg0);
	expr_free(arg1);

	return 0x0;
}

static int expr_array_add_clean(expr_t *arr, expr_t *expr){
	if(expr_array_add(arr, expr) != 0){
		expr_reset(arr);

		return -1;
	}

	return 0;
}

static int attr_reset(type_attr_t *attr, bool clean){
	if(clean && attr->value)
		expr_free(attr->value);

	memset(attr, 0, sizeof(type_attr_t));

	return -1;
}
