#include <arch/syscall.h>
#include <sys/syscall.h>
#include <sys/thread.h>


/* global functions */
void sched_yield(void){
	thread_state_t target;


	target = READY;
	sc(SC_SCHEDYIELD, &target);
}
