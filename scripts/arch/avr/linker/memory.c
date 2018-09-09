/* target header */
#include <config/config.h>
#include <sys/escape.h>

/* host header */
#include <stdio.h>
#include <string.h>
#include <errno.h>


/* local/static prototypes */
static int overlap(char const *name0, size_t base0, size_t size0, char const *name1, size_t base1, size_t size1);


/* global functions */
int main(int argc, char **argv){
	int r;
	char *ofile_name;
	FILE *ofile;


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

	/* check configuration validity */
	r = 0;
	r |= overlap("kernel flash", CONFIG_KERNEL_TEXT_BASE, CONFIG_KERNEL_TEXT_SIZE,
				 "application flash", CONFIG_APP_TEXT_BASE, CONFIG_APP_TEXT_SIZE
	);

	r |= overlap("kernel data", CONFIG_KERNEL_DATA_BASE, CONFIG_KERNEL_DATA_SIZE,
				 "application data", CONFIG_APP_DATA_BASE, CONFIG_APP_DATA_SIZE
	);

	if(r != 0)
		return 1;

	/* generate output file */
	ofile = fopen(ofile_name, "w");

	if(ofile == 0){
		printf("unable to open output file: %s\n", strerror(errno));
		return 1;
	}

	fprintf(ofile, "MEMORY {\n");

	fprintf(ofile, "%20s : ORIGIN = %#10x, LENGTH = %u\n", "flash_kernel", CONFIG_KERNEL_TEXT_BASE, CONFIG_KERNEL_TEXT_SIZE);
	fprintf(ofile, "%20s : ORIGIN = %#10x, LENGTH = %u\n", "flash_app", CONFIG_APP_TEXT_BASE, CONFIG_APP_TEXT_SIZE);
	fprintf(ofile, "%20s : ORIGIN = %#10x, LENGTH = %u\n", "data_kernel", CONFIG_KERNEL_DATA_BASE, CONFIG_KERNEL_DATA_SIZE);
	fprintf(ofile, "%20s : ORIGIN = %#10x, LENGTH = %u\n", "data_app", CONFIG_APP_DATA_BASE, CONFIG_APP_DATA_SIZE);
	fprintf(ofile, "}\n");

	fclose(ofile);

	return 0;
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
