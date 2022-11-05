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
	#include <parser.tab.h>
	#include <stdlib.h>


	/* define YY_USER_ACTION, executed for every token, before excuting its
	 * action code
	 */
	#define YY_USER_ACTION { \
		/* update location based on current token */ \
		devtreelloc.first_line = devtreelloc.last_line = devtree_line; \
		devtreelloc.first_column = devtree_col; \
		devtreelloc.last_column = devtree_col + devtreeleng; \
		devtree_col += devtreeleng; \
	}

	/* make yyunput available outside of the lexer */
	static void yyunput(int c, char *buf_ptr);

	void devtreeunput(char c){
		yyunput(c, (yytext_ptr));
	}
%}

/* start states */
%x MULTI_LINE_COMMENT


%%


%{
	/* code that goes to the beginning of yylex() */
	static unsigned int devtree_col = 1;
	static unsigned int devtree_line = 1;
%}

	/* sections */
"devices"						{ return SEC_DEVICES; }
"memory"						{ return SEC_MEMORY; }

	/* node attributes */
"compatible"					{ return NA_COMPATIBLE; }
"baseaddr"						{ return NA_BASEADDR; }
"reg"							{ return NA_REG; }
"int"							{ return NA_INT; }
"size"							{ return NA_SIZE; }
"string"						{ return NA_STRING; }

	/* comments */
\/\/.*\r?\n						{ devtree_col = 1; ++devtree_line; }
\/\*							{ BEGIN MULTI_LINE_COMMENT; }
<MULTI_LINE_COMMENT>\*\/		{ BEGIN INITIAL; }
<MULTI_LINE_COMMENT>.			{ }
<MULTI_LINE_COMMENT>\r?\n		{ devtree_col = 1; ++devtree_line; }

	/* identifier */
[a-zA-Z][a-zA-Z_\-0-9]+			{ devtreelval.str.s = devtreetext; devtreelval.str.len = devtreeleng; return IDFR; }

	/* string */
\".*\"							{ devtreelval.str.s = devtreetext + 1; devtreelval.str.len = devtreeleng - 2; return STRING; }

	/* integer */
(0x)?[0-9a-fA-F]+				{ devtreelval.i = strtol(devtreetext, 0x0, 0); return INT; }

	/* general patterns */
\r?\n							{ devtree_col = 1; ++devtree_line; }
[ \t]							{ }

	/* default */
.								{ return devtreetext[0]; }
