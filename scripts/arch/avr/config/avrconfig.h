#ifndef AVRCONFIG_H
#define AVRCONFIG_H


#include <stdio.h>


/* macros */
#define CONFIG_PRINT(var, value, fmt) \
	fprintf(arg.ofile, "#define AVRCONFIG_" #var " " fmt "\n", (value))


/* types */
typedef struct{
	FILE *ofile;
	char *ofile_name;
	int verbose;
} arg_t;


/* external variables */
extern arg_t arg;


#endif // AVRCONFIG_H
