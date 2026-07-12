/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef codegen_H
#define codegen_H


#include <stdio.h>
#include <sys/vector.h>


/* prototypes */
void codegen_make(FILE *fp, vector_t *nodes);
void codegen_header(FILE *fp, vector_t *nodes);
void codegen_source(FILE *fp, vector_t *nodes);


#endif // codegen_H
