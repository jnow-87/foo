/**
 * Copyright (C) 2022 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/ctype.h>


/* global functions */
int isalnum(int c){
	return isalpha(c) || isdigit(c);
}

int isalpha(int c){
	return isupper(c) || islower(c);
}

int isblank(int c){
	return c == ' ' || c == '\t';
}

int iscntrl(int c){
	return (c >= 0 && c <= 31) || c == 127;
}

int isdigit(int c){
	return c >= '0' && c <= '9';
}

int isgraph(int c){
	return c >= '!' && c <= '~';
}

int islower(int c){
	return c >= 'a' && c <= 'z';
}

int isprint(int c){
	return c == ' ' || isgraph(c);
}

int ispunct(int c){
	return isprint(c) && !(isspace(c) || isalnum(c));
}

int isspace(int c){
	return c == ' ' || (c >= '\t' && c <= '\r');
}

int isupper(int c){
	return c >= 'A' && c <= 'Z';
}

int isxdigit(int c){
	return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

int tolower(int c){
	if(!isupper(c))
		return c;

	return c + ('a' - 'A');
}

int toupper(int c){
	if(!islower(c))
		return c;

	return c - ('a' - 'A');
}

