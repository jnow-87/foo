#include <arch/syscall.h>
#include <sys/syscall.h>


/* global functions */
void sched_yield(void){
	char dummy;


	(void)sc(SC_SCHEDYIELD, &dummy);
}
