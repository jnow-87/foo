/*
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/io.h>
#include <kernel/kprintf.h>
#include <kernel/opt.h>
#include <sys/stream.h>
#include <sys/stdarg.h>
#include <sys/errno.h>


/* local/static prototypes */
char kputc(char c, FILE *stream);


/* static variables */
FILE kout = FILE_INITIALISER(0x0, 0x0, 0, kputc);


/* global functions */
void kprintf(kmsg_t lvl, char const *format, ...){
	va_list lst;


	va_start(lst, format);
	kvprintf(lvl, format, lst);
	va_end(lst);
}

void kvprintf(kmsg_t lvl, char const *format, va_list lst){
	if((kopt.dbg_lvl & lvl) == 0)
		return;

	vfprintf(&kout, format, lst);
}

/* local functions */
char kputc(char c, FILE *stream){
	return putchar(c);
}
