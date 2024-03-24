#include <arch/arch.h>
#include <kernel/init.h>


/* local functions */
static int init(void){
	// do not reset
	//  - QSPI and XIP since they are needed to fetch code from flash
	//  - PLLs since some clocks might still use them as source, e.g. after a warm reset
	rp2040_resets_halt(~(RP2040_RST_IO_QSPI | RP2040_RST_PADS_QSPI | RP2040_RST_PLL_SYS));
	rp2040_clocks_init();
	rp2040_iomux_init();

	return 0;
}

platform_init(0, first, init);
