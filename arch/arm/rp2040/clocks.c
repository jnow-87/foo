/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arm/rp2040.h>
#include <kernel/kprintf.h>
#include <kernel/stat.h>
#include <sys/math.h>
#include <sys/register.h>
#include <sys/types.h>


/* macros */
// registers
#define FC0_SRC				MREG(CLOCKS_BASE + 0x94)
#define FC0_STATUS			MREG(CLOCKS_BASE + 0x98)
#define FC0_RESULT			MREG(CLOCKS_BASE + 0x9c)
#define FC0_REF_KHZ			MREG(CLOCKS_BASE + 0x80)

#define PLLCS(base)			MREG(base + 0x0)
#define PLLPWR(base)		MREG(base + 0x4)
#define PLLFBDIV(base)		MREG(base + 0x8)
#define PLLPRIM(base)		MREG(base + 0xc)

#define XOSCCTRL			MREG(XOSC_BASE + 0x00)
#define XOSCSTATUS			MREG(XOSC_BASE + 0x04)
#define XOSCDORMANT			MREG(XOSC_BASE + 0x08)
#define XOSCSTARTUP			MREG(XOSC_BASE + 0x0c)
#define XOSCCOUNT			MREG(XOSC_BASE + 0x1c)

#define ROSCCTRL			MREG(ROSC_BASE + 0x0)
#define ROSCSTATUS			MREG(ROSC_BASE + 0x18)

#define CLK_REGS(id)		(CLOCKS_BASE + id * 12)

// register bits
#define FC0_STATUS_RUNNING	8
#define FC0_STATUS_DONE		4

#define FC0_RESULT_KHZ		5
#define FC0_RESULT_FRAC		0

#define PLLCS_LOCK			31
#define PLLCS_BYPASS		8
#define PLLCS_REFDIV		0

#define PLLPWR_VCOPD		5
#define PLLPWR_POSTDIVPD	3
#define PLLPWR_DSMPD		2
#define PLLPWR_PD			0

#define PLLPRIM_POSTDIV1	16
#define PLLPRIM_POSTDIV2	12

#define XOSCCTRL_EN			12
#define XOSCCTRL_FREQRANGE	0

#define XOSCSTATUS_STABLE	31
#define XOSCSTATUS_EN		12

#define XOSCSTARTUP_DELAY	0

#define ROSCCTRL_EN			12
#define ROSCCTRL_FREQRANGE	0

#define ROSCSTATUS_STABLE	31
#define ROSCSTATUS_EN		12

#define CLKCTRL_EN			11
#define CLKCTRL_AUX			5
#define CLKCTRL_SRC			0

#define CLKDIV_INT			8


/* types */
typedef enum{
	CLKSRC_NONE = 0x0,
	CLKSRC_PLL_SYS_CLKSRC,
	CLKSRC_PLL_USB_CLKSRC,
	CLKSRC_ROSC_CLKSRC,
	CLKSRC_ROSC_CLKSRC_PH,
	CLKSRC_XOSC_CLKSRC,
	CLKSRC_CLKSRC_GPIN0,
	CLKSRC_CLKSRC_GPIN1,
	CLKSRC_CLK_REF,
	CLKSRC_CLK_SYS,
	CLKSRC_CLK_PERI,
	CLKSRC_CLK_USB,
	CLKSRC_CLK_ADC,
	CLKSRC_CLK_RTC,
} clk_src_t;

typedef enum{
	CLK_GPOUT0 = 0,
	CLK_GPOUT1,
	CLK_GPOUT2,
	CLK_GPOUT3,
	CLK_REF,
	CLK_SYS,
	CLK_PERI,
	CLK_USB,
	CLK_ADC,
	CLK_RTC,
} clk_id_t;

typedef enum{
	PLL_PWR_NONE = 0x0,
	PLL_PWR_MAIN = (0x1 << PLLPWR_PD),
	PLL_PWR_VCO = (0x1 << PLLPWR_VCOPD),
	PLL_PWR_POSTDIV = (0x1 << PLLPWR_POSTDIVPD),
} pll_power_t;

typedef struct{
	uint32_t volatile ctrl,
					  div,
					  selected;
} clk_regs_t;


/* local/static prototypes */
// clock sources
static void pll_powerup(uint32_t pll_base, pll_power_t en);
static void pll_init(uint32_t pll_base, rp2040_pll_cfg_t *cfg);
static void xosc_init(uint8_t startup_delay_ms, uint32_t ref_clk_khz);
static void xosc_delay(uint32_t f_ref, uint32_t f_delay, uint8_t cycles);
static void rosc_init(bool en);

// clocks
static void clk_set_glitchless_src(clk_id_t id, uint8_t src, uint8_t div);
static void clk_set_glitchless_aux(clk_id_t id, uint8_t aux_src, uint8_t div);
static void clk_set_glitchy_aux(clk_id_t id, uint8_t aux_src, uint8_t div, uint32_t f_khz);

static uint32_t clk_measure(clk_src_t src);


/* global functions */
void rp2040_clocks_init(void){
	uint32_t usb_khz;


	rp2040_platform_cfg_t *cfg = (rp2040_platform_cfg_t*)RP2040_PLATFORM_CONFIG;


	/* NOTE target clock setup
	 * 	ring oscillator (rosc): disabled
	 * 	crystal oscillator (xosc): enabled
	 * 	system pll: configured based on devtree
	 * 	usb pll: configured based on devtree
	 *
	 * 	system clock: system pll
	 * 	reference clock: xosc
	 * 	peripheral clock: xosc
	 * 	usb clock: usb pll
	 * 	adc clock: usb pll
	 * 	rtc clock: xosc
	 */

	/* reset */
	// reset clocks in case the chip was not reset properly, e.g. a debugger left it in an unknown state
	rosc_init(true);
	clk_set_glitchless_src(CLK_REF, 0, 1);
	clk_set_glitchless_src(CLK_SYS, 0, 1);

	/* init clock sources */
	xosc_init(2, cfg->crystal_clock_khz);

	rp2040_resets_release((0x1 << RP2040_RST_PLL_SYS) | (0x1 << RP2040_RST_PLL_USB));
	pll_init(PLL_SYS_BASE, &cfg->pll_sys);
	pll_init(PLL_USB_BASE, &cfg->pll_usb);

	usb_khz = clk_measure(CLKSRC_PLL_USB_CLKSRC);

	/* init clocks */
	clk_set_glitchless_aux(CLK_SYS, 0, 1);
	clk_set_glitchless_src(CLK_REF, 2, 1);

	clk_set_glitchy_aux(CLK_PERI, 4, 1, cfg->crystal_clock_khz);
	clk_set_glitchy_aux(CLK_USB, 0, 1, usb_khz);
	clk_set_glitchy_aux(CLK_ADC, 0, 1, usb_khz);
	clk_set_glitchy_aux(CLK_RTC, 3, 1, cfg->crystal_clock_khz);

	/* disable unused clock sources and clocks */
	rosc_init(false);

	/* set platform info */
	cfg->system_clock_khz = clk_measure(CLKSRC_CLK_SYS);
	cfg->peri_clock_khz = clk_measure(CLKSRC_CLK_PERI);
}


/* local functions */
static void stat(void){
	INFO(
		"clocks\n"
		" ring-osc: %u kHz\n"
		" crystal-osc: %u kHz\n"
		" sys-pll: %u kHz\n"
		" usb-pll: %u kHz\n"
		" sys-clk: %u kHz\n"
		" ref-clk: %u kHz\n"
		" peri-clk: %u kHz\n"
		" usb-clk: %u kHz\n"
		" adc-clk: %u kHz\n"
		" rtc-clk: %u kHz\n"
		, clk_measure(CLKSRC_ROSC_CLKSRC)
		, clk_measure(CLKSRC_XOSC_CLKSRC)
		, clk_measure(CLKSRC_PLL_SYS_CLKSRC)
		, clk_measure(CLKSRC_PLL_USB_CLKSRC)
		, clk_measure(CLKSRC_CLK_SYS)
		, clk_measure(CLKSRC_CLK_REF)
		, clk_measure(CLKSRC_CLK_PERI)
		, clk_measure(CLKSRC_CLK_USB)
		, clk_measure(CLKSRC_CLK_ADC)
		, clk_measure(CLKSRC_CLK_RTC)
	);
}

kernel_stat(stat);

static void pll_powerup(uint32_t pll_base, pll_power_t en){
	PLLPWR(pll_base) = (~en & (PLL_PWR_MAIN | PLL_PWR_VCO | PLL_PWR_POSTDIV)) | (0x1 << PLLPWR_DSMPD);
}

static void pll_init(uint32_t pll_base, rp2040_pll_cfg_t *cfg){
	pll_powerup(pll_base, PLL_PWR_NONE);

	PLLCS(pll_base) = (0x0 << PLLCS_BYPASS) | ((cfg->ref_div & 0x3f) << PLLCS_REFDIV);
	PLLFBDIV(pll_base) = cfg->feedback_div & 0xfff;

	pll_powerup(pll_base, PLL_PWR_MAIN | PLL_PWR_VCO);
	while((PLLCS(pll_base) & (0x1 << PLLCS_LOCK)) == 0);

	PLLPRIM(pll_base) = ((cfg->post_div[0] & 0x7) << PLLPRIM_POSTDIV1) | ((cfg->post_div[1] & 0x7) << PLLPRIM_POSTDIV2);

	pll_powerup(pll_base, PLL_PWR_MAIN | PLL_PWR_VCO | PLL_PWR_POSTDIV);

}

static void xosc_init(uint8_t startup_delay_ms, uint32_t ref_clk_khz){
	XOSCSTARTUP = startup_delay_ms * ref_clk_khz / 256;
	XOSCCTRL = (0xfab << XOSCCTRL_EN) | (0xaa0 << XOSCCTRL_FREQRANGE);

	while((XOSCSTATUS & (0x1 << XOSCSTATUS_STABLE)) == 0);
}

static void xosc_delay(uint32_t f_ref, uint32_t f_delay, uint8_t cycles){
	XOSCCOUNT = (uint8_t)MAX(f_ref / f_delay * cycles, 1u);

	while(XOSCCOUNT != 0);
}

static void rosc_init(bool en){
	ROSCCTRL = ((en ? 0xfab : 0xd1e) << ROSCCTRL_EN) | (0xaa0 << ROSCCTRL_FREQRANGE);

	while(en && (ROSCSTATUS & (0x1 << ROSCSTATUS_STABLE)) == 0);
}

static void clk_set_glitchless_src(clk_id_t id, uint8_t src, uint8_t div){
	clk_regs_t *regs = (clk_regs_t*)CLK_REGS(id);


	regs->ctrl = (regs->ctrl & (0xf << CLKCTRL_AUX)) | (src << CLKCTRL_SRC);
	regs->div = div << CLKDIV_INT;

	while((regs->selected & (0x1 << src)) == 0);
}

static void clk_set_glitchless_aux(clk_id_t id, uint8_t aux_src, uint8_t div){
	clk_regs_t *regs = (clk_regs_t*)CLK_REGS(id);


	clk_set_glitchless_src(id, 0, div); 	// switch src to non-aux
	regs->ctrl |= (aux_src << CLKCTRL_AUX); // set aux
	clk_set_glitchless_src(id, 1, div); 	// switch src to aux
}

static void clk_set_glitchy_aux(clk_id_t id, uint8_t aux_src, uint8_t div, uint32_t f_khz){
	clk_regs_t *regs = (clk_regs_t*)CLK_REGS(id);


	// disable clock
	regs->ctrl = 0;
	regs->div = 0;

	xosc_delay(RP2040_REF_CLOCK_HZ / 1000, f_khz, 2);

	// change aux src
	regs->ctrl = (aux_src << CLKCTRL_AUX);

	// enable clock
	regs->div = (div << CLKDIV_INT);
	regs->ctrl |= (0x1 << CLKCTRL_EN);

	xosc_delay(RP2040_REF_CLOCK_HZ / 1000, f_khz, 2);
}

static uint32_t clk_measure(clk_src_t src){
	FC0_REF_KHZ = RP2040_REF_CLOCK_HZ / 1000;

	while(FC0_STATUS & (0x1 << FC0_STATUS_RUNNING));
	FC0_SRC = src;
	while((FC0_STATUS & (0x1 << FC0_STATUS_DONE)) == 0);

	return FC0_RESULT >> FC0_RESULT_KHZ;
}
