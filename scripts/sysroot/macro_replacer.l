/* options */
%option noyywrap

/* includes */
%{
	#include ARCH_HEADER
	#include <stdio.h>

	#define STR(s)		#s
	#define MACRO(m)	STR(m)
	#define PRINT_MACRO(s)	printf("%s %s\n", #s, STR(s));
%}

/* start states */
%s INCL

%%

"#include"					{ ECHO; BEGIN INCL; }
<INCL>"ARCH_HEADER"			{ printf("%s", MACRO(ARCH_HEADER)); BEGIN INITIAL; }
<INCL>"ARCH_MEM"			{ printf("%s", MACRO(ARCH_MEM)); BEGIN INITIAL; }
<INCL>"ARCH_TIMEBASE"		{ printf("%s", MACRO(ARCH_TIMEBASE)); BEGIN INITIAL; }
<INCL>"ARCH_INTERRUPT"		{ printf("%s", MACRO(ARCH_INTERRUPT)); BEGIN INITIAL; }
<INCL>"ARCH_CORECONFIG"		{ printf("%s", MACRO(ARCH_CORECONFIG)); BEGIN INITIAL; }
<INCL>"ARCH_SPR"			{ printf("%s", MACRO(ARCH_SPR)); BEGIN INITIAL; }
<INCL>"ARCH_SYSCALL"		{ printf("%s", MACRO(ARCH_SYSCALL)); BEGIN INITIAL; }
[.\n]						{ ECHO; BEGIN INITIAL; }


%%

int main(int argc, char** argv){
	FILE* fp;


	if(argc < 2)
		return 1;

	/* print macro definitions */
	if(strcmp(argv[1], "print_macro") == 0){
		PRINT_MACRO(ARCH_HEADER);
		PRINT_MACRO(ARCH_MEM);
		PRINT_MACRO(ARCH_TIMEBASE);
		PRINT_MACRO(ARCH_INTERRUPT);
		PRINT_MACRO(ARCH_CORECONFIG);
		PRINT_MACRO(ARCH_SYSCALL);
		PRINT_MACRO(ARCH_SPR);

		return 0;
	}

	/* replace occurences of '#include ARCH_*' */
	fp = fopen(argv[1], "r");
	if(fp == 0)
		return 1;

	yyrestart(fp);
	yylex();

	fclose(fp);

	return 0;
}
