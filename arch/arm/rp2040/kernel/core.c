#include <arch/arm/v6m.h>
#include <arch/arm/rp2040.h>
#include <kernel/init.h>



/* local functions */
static int init(void){
	return av6m_core_init(RP2040_SYSTEM_CLOCK_HZ / 1000000);
}

// cores have to be initialised after at least the clocks are setup and
// the rp2040 devtree platform configuration has been updated since the
// systick setup relies on those information
platform_init(1, all, init);
