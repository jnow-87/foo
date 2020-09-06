/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>


/* global variables */
void *(*mallocp)(size_t size) = malloc;
void (*freep)(void *addr) = free;
void *(*callocp)(size_t n, size_t size) = calloc;
