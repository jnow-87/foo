/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef OPTIONS_H
#define OPTIONS_H


/* types */
typedef enum{
	FMT_C = 1,
	FMT_HEADER,
	FMT_MAKE,
} output_file_type_t;

typedef enum{
	DT_DRIVER = 0x1,
	DT_MEMORY = 0x2,
	DT_ALL = 0x3
} devtree_section_t;

typedef struct{
	char const *ifile_name;
	char const *ofile_name;

	output_file_type_t ofile_format;
	devtree_section_t export_sections;
} opt_t;


/* external variables */
extern opt_t options;


/* prototypes */
void opt_parse(int argc, char **argv);


#endif // OPTIONS_H
