/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



%option prefix="eval"
%option noyywrap
%option nounput
%option noinput

%{
	#include <eval.tab.h>


	/* macros */
	// define YY_USER_ACTION, executed for every token, before excuting its action code
	#define YY_USER_ACTION { \
		/* update location based on current token */ \
		evallloc.first_line = evallloc.last_line = eval_line; \
		evallloc.first_column = eval_col; \
		evallloc.last_column = eval_col + evalleng; \
		eval_col += evalleng; \
	}

	#define INC_LINE { eval_col = 1; eval_line++; }
%}


%%


%{
	/* code that goes to the beginning of yylex() */
	static unsigned int eval_col = 1;
	static unsigned int eval_line = 1;
%}

	/* identifier */
[a-zA-Z][a-zA-Z_\-0-9]*			{ return eval_parser_strcpy(evallval.s, evaltext, evalleng, IDFR); }

	/* integer */
(0x)?[0-9a-fA-F]+				{ evallval.i = strtol(evaltext, 0x0, 0); return INT; }
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
.								{ return evaltext[0]; }


%%
