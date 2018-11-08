/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef AVRCONFIG_H
#define AVRCONFIG_H


#include <stdio.h>


/* macros */
#define CONFIG_PRINT(var, value, fmt) \
	fprintf(tmp_file, "#define AVRCONFIG_" #var " " fmt "\n", (value))


/* types */
typedef struct{
	char *ofile_name;
	int verbose;
} arg_t;


/* external variables */
extern arg_t arg;
extern FILE *tmp_file;


#endif // AVRCONFIG_H
