#include <arch/syscall.h>
#include <sys/time.h>
#include <sys/syscall.h>


/* global functions */
int time(time_t *t){
	return sc(SC_TIME, t);
}
