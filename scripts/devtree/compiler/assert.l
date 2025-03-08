/**
 * Copyright (C) 2025 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



%option prefix="assert"
%option noinput
%option nounput
%option noyywrap

%{
	#include <stdlib.h>
	#include <stdio.h>
	#include <limits.h>
	#include <asserts.h>
	#include <attr.h>
	#include <parser.tab.h>


	/* prototypes */
	static int assert_parse(assert_t *assert, vector_t *attrs, FILE *fp);
	static int attr_export(FILE *fp, attr_t *attr);
%}


%%


	/* identifier */
[a-zA-Z][a-zA-Z_\-0-9]*			{ return ASSERT_TOKEN; }

	/* default */
.								{ return asserttext[0]; }


%%


int assert_check(assert_t *assert, vector_t *attrs){
	return assert_parse(assert, attrs, 0x0);
}

int assert_export(FILE *fp, assert_t *assert, vector_t *attrs){
	return assert_parse(assert, attrs, fp);
}

static int assert_parse(assert_t *assert, vector_t *attrs, FILE *fp){
	int r = 1;
	char backup;
	attr_t *attr;


	assert_scan_bytes(assert->expr, strlen(assert->expr));

	while(r != 0){
		r = assertlex();

		if(r == ASSERT_TOKEN){
			backup = asserttext[assertleng];
			asserttext[assertleng] = 0;

			attr = attr_get(attrs, asserttext, false);
			asserttext[assertleng] = backup;

			if(attr == 0x0)
				return -1;

			if(fp != 0x0 && attr_export(fp, attr) != 0)
				return -1;
		}
		else if(fp != 0x0 && r > 0 && r <= 255)
			fprintf(fp, "%c", (char)r);
	}

	return 0;
}

// TODO rename export to compile
static int attr_export(FILE *fp, attr_t *attr){
	if(attr->flags & AF_ARRAY){
		// TODO print error
		return -1;
	}

	switch(attr->type){
	case AT_INT8:	// fall through
	case AT_INT16:	// fall through
	case AT_INT32:	// fall through
	case AT_INT64:
		fprintf(fp, "%u", attr->value.i);
		break;

	case AT_ADDR:	fprintf(fp, "(void*)%#x", attr->value.i); break;
	case AT_STRING:	fprintf(fp, "\"%s\"", attr->value.p); break;

	default:
		// TODO error message
		return -1;
	}

	return 0;
}
