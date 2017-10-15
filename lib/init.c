#include <arch/lib.h>
#include <lib/init.h>
#include <sys/compiler.h>


/* prototypes */
int _start(int argc, char **argv) __section(".app_start");


/* extern variables */
extern init_call_t __lib_init_base[],
				   __lib_init_end[];


/* global functions */
int _start(int argc, char **argv){
	init_call_t *p;


	if(lib_crt0() != 0)
		// TODO handle error through exit()
		return -1;

	for(p=__lib_init_base; p<__lib_init_end; p++){
		(void)(*p)();
		// TODO handle error through exit()
	}

	return lib_main(argc, argv);
	// TODO handle return through exit()
	// check current target arch implementations
}
