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
	#include <asserts.h>
	#include <devtree.lex.h>
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

	#define EXPR_ARRAY_ADD(array, expr) \
		EABORT(expr_array_add(array, expr) != 0)

	#define ATTR_ENLIST(attrs, attr) \
		EABORT(attr_enlist(attrs, attr) != 0)

	#define ATTR_ASSIGN(attr, value)({ \
		attr_t *_assi = attr; \
		EABORT(attr_assign(_assi, value) != _assi) \
		_assi; \
	})

	#define ATTR_MATH(a0, a1, op)({ \
		attr_t *_r = attr_math(a0, a1, op, resolve_math); \
		EABORT(_r == 0x0); \
		_r; \
	})

	#define ATTR_ADD(value, op)		ATTR_MATH(value, op, OP_ADD)
	#define ATTR_SUB(value, op)		ATTR_MATH(value, op, OP_SUB)
	#define ATTR_MUL(value, op)		ATTR_MATH(value, op, OP_MUL)
	#define ATTR_DIV(value, op)		ATTR_MATH(value, op, OP_DIV)
	#define ATTR_LSHIFT(value, op)	ATTR_MATH(value, op, OP_LSHIFT)
	#define ATTR_RSHIFT(value, op)	ATTR_MATH(value, op, OP_RSHIFT)
	#define ATTR_MOD(value, op)		ATTR_MATH(value, op, OP_MOD)

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

	#define EXPR(expr, op, arg0, arg1) \
		EABORT(expr_create(expr, op, arg0, arg1) == 0x0)


	/* local/static variables */
	static FILE *fp = 0;
	static char const *dt_script = 0x0;
	static bool erroneous = false;
	static bool resolve_math = false;


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
	#include <expr.h>
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

	if(fp == 0x0){
		fprintf(stderr, "read device tree script \"%s\" failed \"%s\"\n", file, strerror(errno));
		return 1;
	}

	/* start lexer */
	devtreerestart(fp);
}

/* parser union type */
%union{
	expr_t expr;
	ATTR_INT_TYPE i;
	char s[DEVTREE_STRMAX];
	char *sptr;
	new_attr_t *aptr;

	attr_type_t basetype;
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
%type <object> type-body
%type <attr> type-attr
%type <assert> assert

%type <node> node
%type <object> node-args


%type <expr> value

%type <basetype> basetype

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

%type <aptr> attr-ref
%type <expr> array
%type <expr> array-body
%type <expr> const
%type <sptr> string



%%


/* start */
start : devtree											{ if(cleanup() != 0) YYABORT; };

/* sections */
devtree : %empty										{ }
		| error ';'										{ erroneous = true; yyerrok; }
		| devtree ';'									{ }
		| devtree typedef ';'							{ }
		| devtree node ';'								{ EABORT(node_child_add(nodes_root($2->type->category), $2)); }
		;

/* typedef */
typedef : TYPEDEF '{' type-body '}' IDFR				{ EABORT(type_add(STRALLOC($5), &$3.attrs, $3.asserts)); }
		;

type-body : %empty										{ OBJECT_RESET($$); resolve_math = false; }
		  | type-body ';'								{ $$ = $1; }
		  | type-body assert ';'						{ $$ = $1; /*list_add_tail($$.asserts, $2);*/ }
		  | type-body type-attr ';'						{ $$ = $1; ATTR_ENLIST(&$$.attrs, &$2); }
		  | type-body type-attr ASSIGN expression ';'	{ $$ = $1; ATTR_ENLIST(&$$.attrs, ATTR_ASSIGN(&$2, &$4)); }
		  ;

type-attr : basetype IDFR								{ ATTR_INIT(&$$, STRALLOC($2), $1, 0, 0x0); }
		  | basetype IDFR '[' INT ']'					{ ATTR_INIT(&$$, STRALLOC($2), $1, $4, 0x0);}
		  ;

basetype : NA_STRING									{ $$ = AT_STRING; }
		 | NA_ADDR										{ $$ = AT_ADDR; }
		 | NA_INT8   									{ $$ = AT_INT8; }
		 | NA_INT16										{ $$ = AT_INT16; }
		 | NA_INT32										{ $$ = AT_INT32; }
		 | NA_INT64										{ $$ = AT_INT64; }
		 ;

assert : ASSERT '(' expression ',' string ')'			{ /*$$ = assert_create($3, $5); EABORT($$ == 0x0); */ };

/* nodes */
node : IDFR ASSIGN IDFR '(' node-args ')'				{ $$ = type_instantiate(TYPE_LOOKUP($3), STRALLOC($1), &$5.attrs, $5.childs); EABORT($$ == 0x0); }
	 | IDFR ASSIGN IDFR '(' node-args ',' ')'			{ $$ = type_instantiate(TYPE_LOOKUP($3), STRALLOC($1), &$5.attrs, $5.childs); EABORT($$ == 0x0); }
	 ;

node-args : %empty										{ OBJECT_RESET($$); resolve_math = true; devtreeunput(','); }
		  | node-args ',' node							{ $$ = $1; list_add_tail($$.childs, $3); }
		  | node-args ',' IDFR ASSIGN expression		{ $$ = $1; ATTR_ENLIST(&$$.attrs, ATTR_ASSIGN(ATTR_INIT(&(attr_t){}, STRALLOC($3), AT_UNDEF, 0, 0x0), &$5)); }
		  ;

/* expression */
expression : logor										{ $$ = $1; };

logor : logand											{ $$ = $1; }
	  | logor LOGOR logand								{ EXPR(&$$, expr_log_or, &$1, &$3); }
	  ;

logand : bitor											{ $$ = $1; }
	   | logand LOGAND bitor							{ EXPR(&$$, expr_log_and, &$1, &$3); }
	   ;

bitor : bitxor											{ $$ = $1; }
	  | bitor BITOR bitxor								{ EXPR(&$$, expr_bit_or, &$1, &$3); }

bitxor : bitand											{ $$ = $1; }
	   | bitxor BITXOR bitand							{ EXPR(&$$, expr_bit_xor, &$1, &$3); }
	   ;

bitand : equality										{ $$ = $1; }
	   | bitand BITAND equality							{ EXPR(&$$, expr_bit_and, &$1, &$3); }
	   ;

equality : relational									{ $$ = $1; }
		 | equality EQUAL relational					{ EXPR(&$$, expr_eq, &$1, &$3); }
		 | equality UNEQUAL relational					{ EXPR(&$$, expr_neq, &$1, &$3); }
		 ;

relational : shift										{ $$ = $1; }
		   | relational LESSER shift					{ EXPR(&$$, expr_lesser, &$1, &$3); }
		   | relational GREATER shift					{ EXPR(&$$, expr_lesser_eq, &$1, &$3); }
		   | relational LESSEREQ shift					{ EXPR(&$$, expr_greater, &$1, &$3); }
		   | relational GREATEREQ shift					{ EXPR(&$$, expr_greater_eq, &$1, &$3); }
		   ;

shift : additive										{ $$ = $1; }
	  | shift LEFTSHIFT additive						{ EXPR(&$$, expr_lshift, &$1, &$3); }
	  | shift RIGHTSHIFT additive						{ EXPR(&$$, expr_rshift, &$1, &$3); }
	  ;

additive : multiplicative								{ $$ = $1; }
		 | additive PLUS multiplicative					{ EXPR(&$$, expr_add, &$1, &$3); }
		 | additive MINUS multiplicative				{ EXPR(&$$, expr_sub, &$1, &$3); }
		 ;

multiplicative : unary									{ $$ = $1; }
			   | multiplicative MULTIPLY unary			{ EXPR(&$$, expr_mul, &$1, &$3); }
			   | multiplicative DIVIDE unary			{ EXPR(&$$, expr_div, &$1, &$3); }
			   | multiplicative MODULO unary			{ EXPR(&$$, expr_mod, &$1, &$3); }
			   ;

unary : attr-ref UNARYPLUS								{ $$ = $2->value; ATTR_ASSIGN($2, EXPR($2->value, expr_add, $2->value, EXPR_INT(64, 1)); }
	  | attr-ref UNARYMINUS								{ $$ = $2->value; ATTR_ASSIGN($2, EXPR($2->value, expr_sub, $2->value, EXPR_INT(64, 1)); }
	  | UNARYPLUS attr-ref								{ ATTR_ASSIGN($2, EXPR($2->value, expr_add, $2->value, EXPR_INT(64, 1)); $$ = $2->value; }
	  | UNARYMINUS attr-ref								{ ATTR_ASSIGN($2, EXPR($2->value, expr_sub, $2->value, EXPR_INT(64, 1)); $$ = $2->value; }
	  | value											{ $$ = $1; }
	  ;

value : const											{ $$ = $1; }
	  | array											{ $$ = $1; }
	  | attr-ref										{ $$ = $1->expr;   }
	  | '(' expression ')'								{ $$ = $2; }
/*	  | IDFR											{ }  TODO unclear if needed*/
	  ;

/* values */
attr-ref : IDFR '.' IDFR								{ $$ = ATTR_REF($1, $3); };

array : '[' array-body ']'								{ $$ = $2; }
	  | '[' array-body ',' ']'							{ $$ = $2; }
	  ;

array-body : %empty										{ EXPR_ARRAY(ET_UNDEF); devtreeunput(','); }
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
