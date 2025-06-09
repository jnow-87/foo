/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdio.h>
#include <devtree.tab.h>


/* global functions */
int main(int argc, char **argv){
	if(argc < 2)
		return devtree_parser_error("no arg");

	if(devtreeparse(argv[1]) != 0)
		return devtree_parser_error("parser error");

	printf("all good\n");

	return 0;
}
