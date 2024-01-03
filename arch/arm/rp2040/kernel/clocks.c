#include <arch/arch.h>
#include <sys/types.h>
#include <sys/register.h>
#include <sys/math.h>
#include <sys/devtree.h>


/* macros */
#define CLOCK_BASE			0x40008000
#define XOSC_BASE			0x40024000
#define PLL_USB_BASE		0x4002c000
#define PLL_SYS_BASE		0x40028000
#define ROSC_BASE			0x40060000

#define FC0_SRC				MREG(CLOCK_BASE + 0x94)
#define FC0_STATUS			MREG(CLOCK_BASE + 0x98)
#define FC0_RESULT			MREG(CLOCK_BASE + 0x9c)
#define FC0_REF_KHZ			MREG(CLOCK_BASE + 0x80)

#define FC0_STATUS_RUNNING	8
#define FC0_STATUS_DONE		4

#define FC0_RESULT_KHZ		5
#define FC0_RESULT_FRAC		0

#define PLLCS(base)			MREG(base + 0x0)
#define PLLPWR(base)		MREG(base + 0x4)
#define PLLFBDIV(base)		MREG(base + 0x8)
#define PLLPRIM(base)		MREG(base + 0xc)

#define PLLCS_LOCK			31
#define PLLCS_BYPASS		8
#define PLLCS_REFDIV		0

#define PLLPWR_VCOPD		5
#define PLLPWR_POSTDIVPD	3
#define PLLPWR_DSMPD		2
#define PLLPWR_PD			0

#define PLLPRIM_POSTDIV1	16
#define PLLPRIM_POSTDIV2	12

#define XOSCCTRL			MREG(XOSC_BASE + 0x00)
#define XOSCSTATUS			MREG(XOSC_BASE + 0x04)
#define XOSCDORMANT			MREG(XOSC_BASE + 0x08)
#define XOSCSTARTUP			MREG(XOSC_BASE + 0x0c)
#define XOSCCOUNT			MREG(XOSC_BASE + 0x1c)

#define XOSCCTRL_EN			12
#define XOSCCTRL_FREQRANGE	0

#define XOSCSTATUS_STABLE	31
#define XOSCSTATUS_EN		12

#define XOSCSTARTUP_DELAY	0

#define ROSCCTRL			MREG(ROSC_BASE + 0x0)
#define ROSCSTATUS			MREG(ROSC_BASE + 0x18)

#define ROSCCTRL_EN			12
#define ROSCCTRL_FREQRANGE	0

#define ROSCSTATUS_STABLE	31
#define ROSCSTATUS_EN		12

#define CLK_BASE(id)		(CLOCK_BASE + id * 0xc)

#define CLKCTRL(id)			MREG(CLK_BASE(id) + 0x0)
#define CLKDIV(id)			MREG(CLK_BASE(id) + 0x4)
#define CLKSELECTED(id)		MREG(CLK_BASE(id) + 0x8)

#define CLKCTRL_EN			11
#define CLKCTRL_AUX			5
#define CLKCTRL_SRC			0

#define CLKDIV_INT			8


/* types */
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
} clock_id_t;

typedef enum{
	PLL_PWR_NONE = 0x0,
	PLL_PWR_MAIN = (0x1 << PLLPWR_PD),
	PLL_PWR_VCO = (0x1 << PLLPWR_VCOPD),
	PLL_PWR_POSTDIV = (0x1 << PLLPWR_POSTDIVPD),
} pll_power_t;

typedef enum{
	PERI_SRC_SYS = 0,
	PERI_SRC_PLL_SYS,
	PERI_SRC_PLL_USB,
	PERI_SRC_ROSC,
	PERI_SRC_XOSC,
	PERI_SRC_GPIN0,
	PERI_SRC_GPIN1,
} peri_src_t;


/* local/static prototypes */
// clock sources
static void pll_powerup(uint32_t pll_base, pll_power_t en);
static void pll_init(uint32_t pll_base, rp2040_pll_cfg_t *cfg);
static void xosc_init(uint8_t startup_delay_ms, uint32_t ref_clk_khz);
static void xosc_delay(uint32_t f_ref, uint32_t f_delay, uint8_t cycles);
static void rosc_init(bool en);

// clocks
static void clock_set_src(clock_id_t id, uint8_t src, uint8_t aux_src, uint8_t div);
static void clock_set_toaux(clock_id_t id, uint8_t aux_src, uint8_t div);
static void clock_set_aux(clock_id_t id, uint8_t aux_src, uint8_t div, uint32_t f_khz);


/* global functions */
void rp2040_clocks_init(void){
	rp2040_platform_cfg_t *cfg = (rp2040_platform_cfg_t*)RP2040_PLATFORM_CONFIG;


	/* reset */
	// reset clocks in case the chip was not reset properly,
	// e.g. a debugger left it in an unknown state
	rosc_init(true);
	clock_set_src(CLK_REF, 0, 0, 1);
	clock_set_src(CLK_SYS, 0, 0, 1);

	/* init clock sources */
	xosc_init(2, cfg->crystal_clock_khz);

	rp2040_resets_release(RP2040_RST_PLL_SYS | RP2040_RST_PLL_USB);
	pll_init(PLL_SYS_BASE, &cfg->pll_sys);
	pll_init(PLL_USB_BASE, &cfg->pll_usb);

	/* init clocks */
	clock_set_toaux(CLK_SYS, 0, 1);
	clock_set_src(CLK_REF, 2, 0, 1);
	clock_set_src(CLK_USB, 0, 0, 1);

	clock_set_aux(CLK_PERI, 4, 0, cfg->crystal_clock_khz);

	/* disable unused clock sources and clocks */
	rosc_init(false);

	/* set platform info */
	cfg->system_clock_khz = rp2040_clocks_measure(RP2040_CS_CLK_SYS);
	cfg->peri_clock_khz = rp2040_clocks_measure(RP2040_CS_CLK_PERI);
}

uint32_t rp2040_clocks_measure(rp2040_clk_src_t src){
	FC0_REF_KHZ = RP2040_PLATFORM_CONFIG->crystal_clock_khz;

	while(FC0_STATUS & (0x1 << FC0_STATUS_RUNNING));
	FC0_SRC = src;
	while((FC0_STATUS & (0x1 << FC0_STATUS_DONE)) == 0);

	return FC0_RESULT >> FC0_RESULT_KHZ;
}


/* local functions */
static void pll_powerup(uint32_t pll_base, pll_power_t en){
	PLLPWR(pll_base) = (~en & (PLL_PWR_MAIN | PLL_PWR_VCO | PLL_PWR_POSTDIV)) | (0x1 << PLLPWR_DSMPD);
}

static void pll_init(uint32_t pll_base, rp2040_pll_cfg_t *cfg){
	pll_powerup(pll_base, PLL_PWR_NONE);

	PLLCS(pll_base) = (0x0 << PLLCS_BYPASS) | ((cfg->ref_div & 0x3f) << PLLCS_REFDIV);
	PLLFBDIV(pll_base) = cfg->feeback_div & 0xfff;

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

static void clock_set_src(clock_id_t id, uint8_t src, uint8_t aux_src, uint8_t div){
	CLKCTRL(id) = (aux_src << CLKCTRL_AUX) | (src << CLKCTRL_SRC);
	CLKDIV(id) = div << CLKDIV_INT;

	while((CLKSELECTED(id) & (0x1 << src)) == 0);
}

static void clock_set_toaux(clock_id_t id, uint8_t aux_src, uint8_t div){
	// first switch the clock's aux src, keeping the src at a non-aux src (0x0),
	// then set the divider and switch the src to the aux src (0x1)
	CLKCTRL(id) = (aux_src << CLKCTRL_AUX) | (0x0 << CLKCTRL_SRC);
	clock_set_src(id, 1, aux_src, div);
}

static void clock_set_aux(clock_id_t id, uint8_t aux_src, uint8_t div, uint32_t f_khz){
	CLKCTRL(id) = (aux_src << CLKCTRL_AUX);

	if(div != 0)
		CLKDIV(id) = (div << CLKDIV_INT);

	CLKCTRL(id) = (aux_src << CLKCTRL_AUX) | (0x1 << CLKCTRL_EN);

	xosc_delay(RP2040_PLATFORM_CONFIG->crystal_clock_khz, f_khz, 2);
}
