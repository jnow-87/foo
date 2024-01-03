#include <arch/arch.h>
#include <kernel/init.h>
#include <kernel/stat.h>
#include <kernel/kprintf.h>


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

static void stat(void){
	INFO(
		"clocks\n"
		" ring-osc: %u kHz\n"
		" crystal-osc: %u kHz\n"
		" sys-pll: %u kHz\n"
		" usb-pll: %u kHz\n"
		" ref-clk: %u kHz\n"
		" sys-clk: %u kHz\n"
		" peri-clk: %u kHz\n"
		, rp2040_clocks_measure(RP2040_CS_ROSC_CLKSRC)
		, rp2040_clocks_measure(RP2040_CS_XOSC_CLKSRC)
		, rp2040_clocks_measure(RP2040_CS_PLL_SYS_CLKSRC)
		, rp2040_clocks_measure(RP2040_CS_PLL_USB_CLKSRC)
		, rp2040_clocks_measure(RP2040_CS_CLK_REF)
		, rp2040_clocks_measure(RP2040_CS_CLK_SYS)
		, rp2040_clocks_measure(RP2040_CS_CLK_PERI)
	);
}

kernel_stat(stat);
