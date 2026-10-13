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
	#define TYPE_ATTR_ADD(type, attr)				EABORT(type_attr_add_clean(type, attr) != 0)
	#define TYPE_LOOKUP(name)						EABORT_VAL(type_lookup(name), type_t *, == 0x0, _type)

	#define TYPE_INSTANTIATE(type_name, node_name, attrs, childs) \
		EABORT_VAL(type_instantiate_clean(type_name, node_name, attrs, childs), node_t *, == 0x0, _inst)

	#define NODE_ATTR_ADD(node, attr)				EABORT(node_attr_add_clean(node, attr) != 0)
	#define NODE_ATTR_UPDATE(attr_ref, op, expr)	EABORT(node_attr_update(attr_ref.node_p, attr_ref.attr_p, op, expr) != 0)
	#define NODE_REF(name)							EABORT_VAL(node_ref(name), node_t *, == 0x0, _nref)

	#define ASSERT_CREATE(expr, msg)				EABORT_VAL(assert_create(expr, msg), assert_t *, == 0x0, _ass)

	#define EXPR_COPY(dest, src)					EABORT(expr_copy(dest, src) != 0);
	#define EXPR_ARRAY_ADD(array, el)				EABORT(expr_array_add(array, el) != 0);
	#define EXPR_ALLOC(e)							EABORT_VAL(expr_alloc(e), expr_t *, == 0x0, _expr)
	#define EXPR_INIT(expr, op, arg0, arg1)			EABORT_VAL(expr_init(expr, op, arg0, arg1), expr_t *, == 0x0, _eini)

	#define ATTR_ASSIGN(attr, value)				EABORT_VAL(attr_assign(attr, value), attr_t *, == 0x0, _assi)
	#define ATTR_REF(node_ref, attr_name)			EABORT_VAL(attrvec_query(&node_ref->attrs, attr_name, false), attr_t *, == 0x0, _aref)

	#define ATTR_INIT(attr, name, type, array_limit, value) \
		EABORT_VAL(attr_init(attr, name, type, array_limit, value), attr_t *, == 0x0, _aini)

	#define TYPE_CREATE(name, attrs, asserts)		EABORT(type_create_clean(name, attrs, asserts) != 0)

	#define STRALLOC(s)								EABORT_VAL(strmap_add(s), char const *, == 0x0, _s)

	/* static variables */
	static FILE *fp = 0;
	static char const *dt_script = 0x0;
	static bool erroneous = false;


	/* prototypes */
	void devtreeunput(char c);


	/* local/static prototypes */
	static int devtreeerror(char const *file, char const *s);
	static int cleanup(void);

	static expr_t *expr_init(expr_t *expr, expr_op_t op, expr_t *arg0, expr_t *arg1);
	static int type_attr_add_clean(type_t *type, attr_t *attr);
	static int type_create_clean(char const *name, attrvec_t *attrs, assert_t *asserts);
	static int type_reset(type_t *type, bool clean);
	static node_t *type_instantiate_clean(char const *type_name, char const *node_name, attrvec_t *attrs, node_t *childs);

	static int node_attr_add_clean(node_t *node, attr_t *attr);
	static int node_reset(node_t *node, bool clean);

	static int attrvec_add_clean(attrvec_t *attrs, attr_t *attr);
	static int attr_reset(attr_t *attr, bool clean);
	static int expr_reset(expr_t *expr, bool clean);
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
	char const *str_p;
	node_t *node_p;
	assert_t *assert_p;

	struct{
		attr_t *attr_p;
		node_t *node_p;
	} attr_ref;

	type_t type;
	node_t node;
	attr_t attr;
	expr_t expr;
	expr_type_t builtin_type;
	EXPR_INT_T i;
	char s[DEVTREE_STRMAX];
}

%destructor { type_reset(&$$, true); } <type>
%destructor { node_reset(&$$, true); } <node>
%destructor { attr_reset(&$$, true); } <attr>
%destructor { expr_reset(&$$, true); } <expr>

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
typedef : TYPEDEF '{' type-body '}' IDFR				{ TYPE_CREATE(STRALLOC($5), &$3.attrs, $3.asserts); }
		;

type-body : %empty										{ type_reset(&$$, false); }
		  | type-body ';'								{ $$ = $1; }
		  | type-body assert ';'						{ $$ = $1; list_add_tail($$.asserts, $2); }
		  | type-body type-attr ';'						{ $$ = $1; TYPE_ATTR_ADD(&$$, &$2); }
		  | type-body type-attr ASSIGN expression ';'	{ $$ = $1; TYPE_ATTR_ADD(&$$, ATTR_ASSIGN(&$2, EXPR_ALLOC(&$4))); }
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
node : IDFR ASSIGN IDFR '(' node-args ')'				{ $$ = TYPE_INSTANTIATE($3, STRALLOC($1), &$5.attrs, $5.childs); }
	 | IDFR ASSIGN IDFR '(' node-args ',' ')'			{ $$ = TYPE_INSTANTIATE($3, STRALLOC($1), &$5.attrs, $5.childs); }
	 ;

node-args : %empty										{ node_reset(&$$, false); devtreeunput(','); }
		  | node-args ',' node							{ $$ = $1; list_add_tail($$.childs, $3); }
		  | node-args ',' IDFR ASSIGN expression		{ $$ = $1; NODE_ATTR_ADD(&$$, ATTR_INIT(&(attr_t){ }, STRALLOC($3), expr_type(&$5), 0, EXPR_ALLOC(&$5))); }
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
unary : attr-ref UNARYPLUS								{ $$ = *($1.attr_p->value); NODE_ATTR_UPDATE($1, EOP_ADD, &EXPR_INT(64, 1)); }
	  | UNARYPLUS attr-ref								{ NODE_ATTR_UPDATE($2, EOP_ADD, &EXPR_INT(64, 1)); $$ = *($2.attr_p->value); }
	  | value											{ $$ = $1; }
	  ;

value : const											{ $$ = $1; }
	  | array											{ $$ = $1; }
	  | attr-ref										{ EXPR_COPY(&$$, $1.attr_p->value); }
	  | '(' expression ')'								{ $$ = $2; }
	  | IDFR											{ $$ = EXPR_REF(STRALLOC($1)); }
	  ;

/* values */
attr-ref : IDFR '.' IDFR								{ $$.node_p = NODE_REF($1); $$.attr_p = ATTR_REF($$.node_p, $3); };

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

		*expr = EXPR(op, EXPR_VALUE_EXPR(arg0), EXPR_VALUE_EXPR(arg1));
	}
	else{
		*expr = EXPR_LITERAL(r);
		expr_value_free(&arg0->arg0);
		expr_value_free(&arg1->arg0);

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

static int type_attr_add_clean(type_t *type, attr_t *attr){
	if(attrvec_add_clean(&type->attrs, attr) != 0){
		asserts_destroy(type->asserts);

		return -1;
	}

	return 0;
}


static int attrvec_add_clean(attrvec_t *attrs, attr_t *attr){
	if(attrvec_add(attrs, attr) != 0){
		if(attr->value)
			expr_free(attr->value);

		attrvec_destroy(attrs);

		return -1;
	}

	return 0;
}

static int type_create_clean(char const *name, attrvec_t *attrs, assert_t *asserts){
	if(type_create(name, attrs, asserts) != 0)
		return type_reset(&(type_t){ .attrs = *attrs, .asserts = asserts }, true);

	return 0;
}

static int type_reset(type_t *type, bool clean){
	if(clean){
		asserts_destroy(type->asserts);
		attrvec_destroy(&type->attrs);
	}

	type->attrs = ATTRVEC_INITIALISER();
	type->asserts = 0x0;

	return -1;
}

static node_t *type_instantiate_clean(char const *type_name, char const *node_name, attrvec_t *attrs, node_t *childs){
	node_t *n = 0x0;
	type_t *t;


	t = type_lookup(type_name);

	if(t != 0x0)
		n = type_instantiate(t, node_name, attrs, childs);

	attrvec_destroy(attrs); /* TODO avoid freeing the entire vector, only reset it */

	return n;
}

static int node_attr_add_clean(node_t *node, attr_t *attr){
	return attrvec_add_clean(&node->attrs, attr);
}

static int node_reset(node_t *node, bool clean){
	if(clean)
		attrvec_destroy(&node->attrs);

	node->attrs = ATTRVEC_INITIALISER();
	node->childs = 0x0;

	return -1;
}

static int attr_reset(attr_t *attr, bool clean){
	if(clean && attr->value)
		expr_free(attr->value);

	memset(attr, 0, sizeof(attr_t));

	return -1;
}

static int expr_reset(expr_t *expr, bool clean){
	if(clean){
		expr_value_free(&expr->arg0);
		expr_value_free(&expr->arg1);
	}

	memset(expr, 0, sizeof(expr_t));

	return -1;
}
