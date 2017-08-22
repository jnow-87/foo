#include <arch/lib.h>
#include <sys/compiler.h>


/* prototypes */
int _start(int argc, char **argv) __section(".app_start");


/* global functions */
int _start(int argc, char **argv){
	if(lib_init() != 0)
		return -1;

	return lib_main(argc, argv);
}
