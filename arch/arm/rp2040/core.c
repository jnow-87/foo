#include <arch/arch.h>
#include <sys/register.h>



#define SIO_BASE		0xd0000000

#define SIO_CPUID		MREG(SIO_BASE | 0x000)


/* global functions */
int rp2040_core_id(void){
	return SIO_CPUID;
}
