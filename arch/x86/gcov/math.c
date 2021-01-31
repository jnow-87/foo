/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/compiler.h>


/* prototypes */
int gcov___popcountdi2(unsigned long a);
int gc_popcntdi2_(unsigned long a) __alias(gcov___popcountdi2);


/* global functions */
int gcov___popcountdi2(unsigned long a){
	return __builtin_popcount(a);
}
