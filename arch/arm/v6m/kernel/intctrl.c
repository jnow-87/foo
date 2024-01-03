#include <arch/arm/v6m.h>
#include <kernel/interrupt.h>


#define PRIMASK_PM	0


bool av6m_int_enable(bool en){
	bool s;


	s = av6m_int_enabled();

	if(en)	asm volatile("cpsie i");
	else	asm volatile("cpsid i");

	return s;
}

bool av6m_int_enabled(void){
	return ((mrs(primask) & (0x1 << PRIMASK_PM)) == 0);
}
