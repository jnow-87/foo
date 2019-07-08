/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



/* target header */
#include <config/config.h>
#include <sys/devtree.h>
#include <sys/escape.h>

/* host header */
#include <stdio.h>
#include <string.h>
#include <errno.h>


/* local/static prototypes */
static int overlap(char const *name0, size_t base0, size_t size0, char const *name1, size_t base1, size_t size1);


/* global functions */
int main(int argc, char **argv){
	char *ofile_name;
	FILE *ofile;
	unsigned int i;
	int r;
	devtree_memory_t const *node;
	char const *sec_names[] = {
		"kernel_flash",
		"app_flash",
		"kernel_data",
		"app_data",
	};


	/* check arguments */
	if(argc < 2){
		printf("usage: %s <output>\n\n"
			   "%15s    %s\n"
			   ,
			   argv[0],
			   "<output>", "output linker file name"
		);

		return 1;
	}

	ofile_name = argv[1];

	printf("generating avr memory layout linker script \"%s\"\n", ofile_name);

	/* generate output file */
	ofile = fopen(ofile_name, "w");

	if(ofile == 0){
		printf("open output file failed \"%s\"\n", strerror(errno));
		return 1;
	}

	fprintf(ofile, "MEMORY {\n");

	r = 0;

	for(i=0; i<sizeof(sec_names)/sizeof(*sec_names); i++){
		node = devtree_find_memory_by_name(&__dt_memory_root, sec_names[i]);

		if(node == 0x0)
			r = 1;

		fprintf(ofile, "%20s : ORIGIN = %#10lx, LENGTH = %u\n", sec_names[i], (unsigned long int)node->base, node->size);
	}

	fprintf(ofile, "}\n");

	fclose(ofile);

	return r;
}


/* local functions */
/**
 * \brief	check if two sections have some overlap
 * 			sections are defined by their base address and size
 *
 * \param	name0		name of first section
 * \param	base0		base address of the first section
 * \param	size0		size of the first section
 * \param	name0		name of second section
 * \param	base0		base address of the second section
 * \param	size0		size of the second section
 *
 * \return	0	no overlap detected
 * 			1	overlap detected
 */
static int overlap(char const *name0, size_t base0, size_t size0, char const *name1, size_t base1, size_t size1){
	if(base1 >= base0 + size0 || base0 >= base1 + size1)
		return 0;

	printf("\terror: %s and %s regions overlap, check config\n\n", name0, name1);

	printf(BG_WHITE FG_BLACK "         section         base          end        size               " RESET_ATTR "\n");
	printf(BOLD "%20.20s" RESET_ATTR ": 0x%8.8lx - 0x%8.8lx %8lu\n", name0, base0, base0 + size0 - 1, size0);
	printf(BOLD "%20.20s" RESET_ATTR ": 0x%8.8lx - 0x%8.8lx %8lu\n", name1, base1, base1 + size1 - 1, size1);
	printf("\n");

	return -1;
}
