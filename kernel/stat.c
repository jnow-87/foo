#include <kernel/opt.h>
#include <kernel/stat.h>


/* external variables */
extern stat_call_t __kernel_stat_base[],
				   __kernel_stat_end[];


/* global functions */
void kstat(){
	stat_call_t *p;


	if(!kopt.kernel_stat)
		return;

	for(p=__kernel_stat_base; p<__kernel_stat_end; p++)
		(*p)();
}
