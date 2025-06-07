/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



%define api.prefix {eval}
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
	#include <eval.lex.h>
	#include <nodes.h>
	#include <types.h>


	/* macros */
	#define YYDEBUG	1

	// parser error message
	#define EABORT(expr){ \
		if(expr) \
			YYERROR; \
	}


	/* local/static prototypes */
	static int evalerror(char const *file, char const *s);
	static void cleanup(void);
%}

%code requires{
	#include <sys/list.h>
	#include <sys/vector.h>
	#include <asserts.h>
	#include <nodes.h>
	#include <types.h>


	/* macros */
	#define EVAL_STRMAX	64


	/* prototypes */
	int eval_parser_error(char const *fmt, ...);
	int eval_parser_strcpy(char *dst, char const *src, size_t n, int token);
}

/* parse paramters */
%parse-param { char const *expr }

/* init code */
%initial-action{
	/* start lexer */
	eval_scan_bytes(expr, strlen(expr));
}

/* parser union type */
%union{
	ATTR_INT_TYPE i;
	char s[EVAL_STRMAX];
}

/* terminals */
// general
%token <i> INT
%token <s> IDFR

%token EQUAL
%token UNEQUAL
%token LESSER
%token GREATER
%token LESSEREQ
%token GREATEREQ
%token LEFTSHIFT
%token RIGHTSHIFT
%token PLUS
%token MINUS
%token MULTIPLY
%token DIVIDE
%token MODULO

%type <i> expression
//%type <i> rel
//%type <i> rel-op
//%type <i> shift
//%type <i> shift-op
//%type <i> add
//%type <i> add-op


%type <i> equality
%type <i> relational
%type <i> shift
%type <i> additive
%type <i> multiplicative


%type <i> op


%%


/* start */
start : expression					{ printf("result: %d\n", $1); cleanup(); };



expression : equality {};
equality : relational					{ }
	  | equality EQUAL relational	{ }
	  | equality UNEQUAL relational	{ }
	  ;

relational : shift					{ }
		   | relational LESSER shift	{ }
		   | relational GREATER shift	{ }
		   | relational LESSEREQ shift	{ }
		   | relational GREATEREQ shift	{ }
		   ;

shift : additive					{ }
	  | shift LEFTSHIFT additive		{ $$ = $1 << $3; }
	  | shift RIGHTSHIFT additive		{ $$ = $1 >> $3; }
	  ;

additive : multiplicative			{ }
		 | additive PLUS multiplicative	{ $$ = $1 + $3; }
		 | additive MINUS multiplicative	{ $$ = $1 - $3; }
		 ;

multiplicative : op					{ }
			   | multiplicative MULTIPLY op	{ $$ = $1 * $3; }
			   | multiplicative DIVIDE op	{ $$ = $1 / $3; }
			   | multiplicative MODULO op	{ $$ = $1 % $3; }
			   ;

op : INT							{ $$ = $1; }
   | '(' expression ')'					{ $$ = $2; }
   ;


%%


/* global functions */
int eval_parser_error(char const *fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	vfprintf(stderr, fmt, lst);
	va_end(lst);

	fprintf(stderr, " %s\n", (errno ? strerror(errno) : ""));

	return -1;
}

int eval_parser_strcpy(char *dst, char const *src, size_t n, int token){
	if(n >= EVAL_STRMAX){
		eval_parser_error("string too long, max=%u", EVAL_STRMAX);

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
static int evalerror(char const *file, char const *s){
	eval_parser_error(s);

	return 0;
}

static void cleanup(void){
	evallex_destroy();
}
