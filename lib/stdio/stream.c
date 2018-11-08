/*
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <lib/stdio.h>
#include <sys/stdarg.h>
#include <sys/stream.h>
#include <sys/limits.h>


/* global functions */
int sprintf(char *s, char const *format, ...){
	int i;
	va_list lst;


	va_start(lst, format);
	i = vsprintf(s, format, lst);
	va_end(lst);

	return i;
}

int snprintf(char *s, size_t n, char const *format, ...){
	int i;
	va_list lst;


	va_start(lst, format);
	i = vsnprintf(s, n, format, lst);
	va_end(lst);

	return i;
}

int vsprintf(char *s, char const *format, va_list lst){
	return vsnprintf(s, SIZE_MAX, format, lst);
}

int vsnprintf(char *s, size_t n, char const *format, va_list lst){
	FILE fp = FILE_INITIALISER(0x0, s, n, 0x0);


	return vfprintf(&fp, format, lst);
}
