/* target header */
#include BUILD_ARCH_HEADER
#include <config/config.h>
#include <sys/compiler.h>

/* host header */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>


/* global functions */
int main(int argc, char **argv){
	char *ofile_name;
	char *bin_type;
	char c;
	FILE *ofile;
	union{
		unsigned int i;
		char c[4];
	} filler;

	/* check arguments */
	if(argc < 3){
		printf("usage: %s <output> <bin type>\n\n"
			   "%15s    %s\n"
			   "%15s    %s\n"
			   ,
			   argv[0],
			   "<output>", "output linker file name",
			   "<bin type>", "target binary for output file (kernel or app)"
		);

		return 1;
	}

	ofile_name = argv[1];
	bin_type = argv[2];

	printf("generating avr %s plugin linker script \"%s\"\n", bin_type, ofile_name);

	/* generate output file */
	ofile = fopen(ofile_name, "w");

	if(ofile == 0){
		printf("unable to open output file: %s\n", strerror(errno));
		return 1;
	}

	// flash fill: fills unsused flash areas with jumps to an overflow handler
	if(strcmp(STR(CONFIG_BUILD_TYPE), "debug") == 0){
		filler.i = 0x940e0000
				 | ((CONFIG_KERNEL_TEXT_BASE + (INT_VECTORS + 1) * INT_VEC_SIZE) / 2);

		filler.i = htonl(filler.i);

		c = filler.c[1];
		filler.c[1] = filler.c[0];
		filler.c[0] = c;
		c = filler.c[3];
		filler.c[3] = filler.c[2];
		filler.c[2] = c;

		fprintf(ofile, ".fill : {\n");
		fprintf(ofile, "\t. = ALIGN(2);\n");
		fprintf(ofile, "\tFILL(0x%.2hhx%.2hhx%.2hhx%.2hhx);\n", filler.c[0], filler.c[1], filler.c[2], filler.c[3]);
		fprintf(ofile, "\t. = ORIGIN(flash_%s) + LENGTH(flash_%s) - 4;\n", bin_type, bin_type);
		fprintf(ofile, "\tBYTE(0x%.2hhx);\n", filler.c[0]);
		fprintf(ofile, "\tBYTE(0x%.2hhx);\n", filler.c[1]);
		fprintf(ofile, "\tBYTE(0x%.2hhx);\n", filler.c[2]);
		fprintf(ofile, "\tBYTE(0x%.2hhx);\n", filler.c[3]);
		fprintf(ofile, "} > flash_%s\n", bin_type);
	}

	fclose(ofile);

	return 0;
}
