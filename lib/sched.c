#include <arch/syscall.h>
#include <sys/syscall.h>


/* global functions */
void sched_yield(void){
	int p;


	sc(SC_SCHEDYIELD, &p);
}
