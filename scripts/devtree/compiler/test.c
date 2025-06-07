/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <eval.tab.h>


/* global functions */
int main(int argc, char **argv){
	if(argc < 2)
		return eval_parser_error("no arg");

	if(evalparse(argv[1]) != 0)
		return eval_parser_error("parser error");

	printf("all good\n");

	return 0;
}
