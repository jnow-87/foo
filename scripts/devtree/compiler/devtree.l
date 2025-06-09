/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



%option prefix="devtree"
%option noyywrap
%option noinput

%{
	#include <stdlib.h>
	#include <devtree.tab.h>


	/* macros */
	// define YY_USER_ACTION, executed for every token, before excuting its action code
	#define YY_USER_ACTION { \
		/* update location based on current token */ \
		devtreelloc.first_line = devtreelloc.last_line = devtree_line; \
		devtreelloc.first_column = devtree_col; \
		devtreelloc.last_column = devtree_col + devtreeleng; \
		devtree_col += devtreeleng; \
	}

	#define INC_LINE { devtree_col = 1; devtree_line++; }


	/* local/static prototypes */
	// make yyunput available outside of the lexer
	static void yyunput(int c, char *buf_ptr);
%}

/* start states */
%x MULTI_LINE_COMMENT


%%


%{
	/* code that goes to the beginning of yylex() */
	static unsigned int devtree_col = 1;
	static unsigned int devtree_line = 1;
%}

	/* typedef */
"memorytype"					{ return TYPEDEF_MEM; }
"devicetype"					{ return TYPEDEF_DEV; }

	/* node attributes */
"addr"							{ return NA_ADDR; }
"int8"							{ return NA_INT8; }
"int16"							{ return NA_INT16; }
"int32"							{ return NA_INT32; }
"int64"							{ return NA_INT64; }
"string"						{ return NA_STRING; }

	/* asserts */
"assert"						{ return ASSERT; }

	/* comments */
\/\/.*\r?\n						{ INC_LINE }
\/\*							{ BEGIN MULTI_LINE_COMMENT; }
<MULTI_LINE_COMMENT>\*\/		{ BEGIN INITIAL; }
<MULTI_LINE_COMMENT>.			{ }
<MULTI_LINE_COMMENT>\r?\n		{ INC_LINE }

	/* identifier */
[a-zA-Z][a-zA-Z_\-0-9]*			{ return devtree_parser_strcpy(devtreelval.s, devtreetext, devtreeleng, IDFR); }

	/* string */
\"[^"]*\"						{ return devtree_parser_strcpy(devtreelval.s, devtreetext + 1, devtreeleng - 2, STRING); }

	/* integer */
(0x)?[0-9a-fA-F]+				{ devtreelval.i = strtol(devtreetext, 0x0, 0); return INT; }

	/* operators */
==	{ return EQUAL; }
!= { return UNEQUAL; }
\<	{ return LESSER; }
>	{ return GREATER; }
\<=	{ return LESSEREQ; }
>=	{ return GREATEREQ; }
\<<	{ return LEFTSHIFT; }
>>	{ return RIGHTSHIFT; }
\+	{ return PLUS; }
-	{ return MINUS; }
\*	{ return MULTIPLY; }
\/	{ return DIVIDE; }
%	{ return MODULO; }


	/* general patterns */
\r?\n							{ INC_LINE }
[ \t]							{ }

	/* default */
.								{ return devtreetext[0]; }


%%


void devtreeunput(char c){
	yyunput(c, (yytext_ptr));
}
