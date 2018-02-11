#include <arch/syscall.h>
#include <sys/syscall.h>


/* global functions */
void exit(int status){
	(void)sc(SC_EXIT, &status);
}
