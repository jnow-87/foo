/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/string.h>


/* global functions */
long __isoc23_strtol(const char *p, char **endp, int base){
	return strtol(p, endp, base);
}

