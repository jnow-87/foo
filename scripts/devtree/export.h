/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef EXPORT_H
#define EXPORT_H


#include <stdio.h>


/* prototypes */
void export_make(FILE *fp);
void export_header(FILE *fp);
void export_source(FILE *fp);


#endif // EXPORT_H
