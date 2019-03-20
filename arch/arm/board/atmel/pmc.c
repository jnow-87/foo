/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <sys/errno.h>
#include <sys/register.h>


/* macros */
#define PMC_BASE		0x400e0600

// PMC registers
#define SCER			(PMC_BASE | 0x000)
#define SCDR			(PMC_BASE | 0x004)
#define SCSR			(PMC_BASE | 0x008)
#define PCER0			(PMC_BASE | 0x010)
#define PCDR0			(PMC_BASE | 0x014)
#define PCSR0			(PMC_BASE | 0x018)
#define UCKR			(PMC_BASE | 0x01c)
#define MOR				(PMC_BASE | 0x020)
#define MCFR			(PMC_BASE | 0x024)
#define PLLAR			(PMC_BASE | 0x028)
#define MCKR			(PMC_BASE | 0x030)
#define USB				(PMC_BASE | 0x038)
#define PCK(x)			(PMC_BASE | (0x040 + (x) * 0x4))
#define IER				(PMC_BASE | 0x060)
#define IDR				(PMC_BASE | 0x064)
#define SR				(PMC_BASE | 0x068)
#define IMR				(PMC_BASE | 0x06c)
#define FSMR			(PMC_BASE | 0x070)
#define FSPR			(PMC_BASE | 0x074)
#define FOCR			(PMC_BASE | 0x078)
#define WPMR			(PMC_BASE | 0x0e4)
#define WPSR			(PMC_BASE | 0x0e8)
#define PCER1			(PMC_BASE | 0x100)
#define PCDR1			(PMC_BASE | 0x104)
#define PCSR1			(PMC_BASE | 0x108)
#define PCR				(PMC_BASE | 0x10c)
#define OCR				(PMC_BASE | 0x110)
#define SLPWKER0		(PMC_BASE | 0x114)
#define SLPWKDR0		(PMC_BASE | 0x118)
#define SLPWKSR0		(PMC_BASE | 0x11c)
#define SLPWKASR0		(PMC_BASE | 0x120)
#define PMMR			(PMC_BASE | 0x130)
#define SLPWKER1		(PMC_BASE | 0x134)
#define SLPWKDR1		(PMC_BASE | 0x138)
#define SLPWKSR1		(PMC_BASE | 0x13c)
#define SLPWKASR1		(PMC_BASE | 0x140)
#define SLPWKAIPR		(PMC_BASE | 0x144)

// PMC bits
#define SR_XT32KERR		21
#define SR_FOS			20
#define SR_CFDS			19
#define SR_CFDEV		18
#define SR_MOSCRCS		17
#define SR_MOSCSELS		16
#define SR_PCKRDY		8
#define SR_OSCSELS		7
#define SR_LOCKU		6
#define SR_MCKRDY		3
#define SR_LOCKA		1
#define SR_MOSCXTS		0

#define WPMR_WPKEY		8
#define WPMR_WPEN		0

#define WPSR_WPVSRC		8
#define WPSR_WPVS		0

#define MOR_XT32KFME	26
#define MOR_CFDEN		25
#define MOR_MOSCSEL		24
#define MOR_KEY			16
#define MOR_MOSCXTST	8
#define MOR_MOSCRCF		4
#define MOR_MOSCRCEN	3
#define MOR_WAITMODE	2
#define MOR_MOSCXTBY	1
#define MOR_MOSCXTEN	0

#define MCFR_CCSS		24
#define MCFR_RCMEAS		20
#define MCFR_MAINFRDY	16
#define MCFR_MAINF		0

#define OCR_SEL12		23
#define OCR_CAL12		16
#define OCR_SEL8		15
#define OCR_CAL8		8
#define OCR_SEL4		7
#define OCR_CAL4		0

#define FOCR_FOCLR		0

#define PLLAR_ONE		29
#define PLLAR_MULA		16
#define PLLAR_PLLACOUNT	8
#define PLLAR_DIVA		0

#define PMMR_MAX		0

#define UCKR_UPLLCOUNT	20
#define UCKR_UPLLEN		16

#define MCKR_UPLLDIV2	13
#define MCKR_MDIV		8
#define MCKR_PRES		4
#define MCKR_CSS		0

#define FSMR_FFLPM		23
#define FSMR_FLPM		21
#define FSMR_LPM		20
#define FSMR_USBAL		18
#define FSMR_RTCAL		17
#define FSMR_RTTAL		16
#define FSMR_FSTT		0

#define FSPR_FSTP		0

#define PCR_GCLKEN		29
#define PCR_EN			28
#define PCR_GCLKDIV		20
#define PCR_CMD			12
#define PCR_GCLKCSS		8
#define PCR_PID			0

#define PCER0_PID31		31
#define PCER0_PID30		30
#define PCER0_PID29		29
#define PCER0_PID28		28
#define PCER0_PID27		27
#define PCER0_PID26		26
#define PCER0_PID25		25
#define PCER0_PID24		24
#define PCER0_PID23		23
#define PCER0_PID22		22
#define PCER0_PID21		21
#define PCER0_PID20		20
#define PCER0_PID19		19
#define PCER0_PID18		18
#define PCER0_PID17		17
#define PCER0_PID16		16
#define PCER0_PID15		15
#define PCER0_PID14		14
#define PCER0_PID13		13
#define PCER0_PID12		12
#define PCER0_PID11		11
#define PCER0_PID10		10
#define PCER0_PID9		9
#define PCER0_PID8		8
#define PCER0_PID7		7

#define PCDR0_PID31		31
#define PCDR0_PID30		30
#define PCDR0_PID29		29
#define PCDR0_PID28		28
#define PCDR0_PID27		27
#define PCDR0_PID26		26
#define PCDR0_PID25		25
#define PCDR0_PID24		24
#define PCDR0_PID23		23
#define PCDR0_PID22		22
#define PCDR0_PID21		21
#define PCDR0_PID20		20
#define PCDR0_PID19		19
#define PCDR0_PID18		18
#define PCDR0_PID17		17
#define PCDR0_PID16		16
#define PCDR0_PID15		15
#define PCDR0_PID14		14
#define PCDR0_PID13		13
#define PCDR0_PID12		12
#define PCDR0_PID11		11
#define PCDR0_PID10		10
#define PCDR0_PID9		9
#define PCDR0_PID8		8
#define PCDR0_PID7		7

#define PCSR0_PID31		31
#define PCSR0_PID30		30
#define PCSR0_PID29		29
#define PCSR0_PID28		28
#define PCSR0_PID27		27
#define PCSR0_PID26		26
#define PCSR0_PID25		25
#define PCSR0_PID24		24
#define PCSR0_PID23		23
#define PCSR0_PID22		22
#define PCSR0_PID21		21
#define PCSR0_PID20		20
#define PCSR0_PID19		19
#define PCSR0_PID18		18
#define PCSR0_PID17		17
#define PCSR0_PID16		16
#define PCSR0_PID15		15
#define PCSR0_PID14		14
#define PCSR0_PID13		13
#define PCSR0_PID12		12
#define PCSR0_PID11		11
#define PCSR0_PID10		10
#define PCSR0_PID9		9
#define PCSR0_PID8		8
#define PCSR0_PID7		7

#define PCER1_PID62		30
#define PCER1_PID60		28
#define PCER1_PID59		27
#define PCER1_PID58		26
#define PCER1_PID57		25
#define PCER1_PID56		24
#define PCER1_PID53		21
#define PCER1_PID52		20
#define PCER1_PID51		19
#define PCER1_PID50		18
#define PCER1_PID49		17
#define PCER1_PID48		16
#define PCER1_PID47		15
#define PCER1_PID46		14
#define PCER1_PID45		13
#define PCER1_PID44		12
#define PCER1_PID43		11
#define PCER1_PID42		10
#define PCER1_PID41		9
#define PCER1_PID40		8
#define PCER1_PID39		7
#define PCER1_PID37		5
#define PCER1_PID35		3
#define PCER1_PID34		2
#define PCER1_PID33		1
#define PCER1_PID32		0

#define PCDR1_PID62		30
#define PCDR1_PID60		28
#define PCDR1_PID59		27
#define PCDR1_PID58		26
#define PCDR1_PID57		25
#define PCDR1_PID56		24
#define PCDR1_PID53		21
#define PCDR1_PID52		20
#define PCDR1_PID51		19
#define PCDR1_PID50		18
#define PCDR1_PID49		17
#define PCDR1_PID48		16
#define PCDR1_PID47		15
#define PCDR1_PID46		14
#define PCDR1_PID45		13
#define PCDR1_PID44		12
#define PCDR1_PID43		11
#define PCDR1_PID42		10
#define PCDR1_PID41		9
#define PCDR1_PID40		8
#define PCDR1_PID39		7
#define PCDR1_PID37		5
#define PCDR1_PID35		3
#define PCDR1_PID34		2
#define PCDR1_PID33		1
#define PCDR1_PID32		0

#define PCSR1_PID62		30
#define PCSR1_PID60		28
#define PCSR1_PID59		27
#define PCSR1_PID58		26
#define PCSR1_PID57		25
#define PCSR1_PID56		24
#define PCSR1_PID53		21
#define PCSR1_PID52		20
#define PCSR1_PID51		19
#define PCSR1_PID50		18
#define PCSR1_PID49		17
#define PCSR1_PID48		16
#define PCSR1_PID47		15
#define PCSR1_PID46		14
#define PCSR1_PID45		13
#define PCSR1_PID44		12
#define PCSR1_PID43		11
#define PCSR1_PID42		10
#define PCSR1_PID41		9
#define PCSR1_PID40		8
#define PCSR1_PID39		7
#define PCSR1_PID37		5
#define PCSR1_PID35		3
#define PCSR1_PID34		2
#define PCSR1_PID33		1
#define PCSR1_PID32		0

#define PCKx_PRES		4
#define PCKx_CSS		0

#define SCER_PCK7		15
#define SCER_PCK6		14
#define SCER_PCK5		13
#define SCER_PCK4		12
#define SCER_PCK3		11
#define SCER_PCK2		10
#define SCER_PCK1		9
#define SCER_PCK0		8
#define SCER_USBCLK		5

#define SCDR_PCK7		15
#define SCDR_PCK6		14
#define SCDR_PCK5		13
#define SCDR_PCK4		12
#define SCDR_PCK3		11
#define SCDR_PCK2		10
#define SCDR_PCK1		9
#define SCDR_PCK0		8
#define SCDR_USBCLK		5

#define SCSR_PCK7		15
#define SCSR_PCK6		14
#define SCSR_PCK5		13
#define SCSR_PCK4		12
#define SCSR_PCK3		11
#define SCSR_PCK2		10
#define SCSR_PCK1		9
#define SCSR_PCK0		8
#define SCSR_USBCLK		5
#define SCSR_HCLKS		0

#define USB_USBDIV		8
#define USB_USBS		0

#define IER_XT32KERR	21
#define IER_CFDEV		18
#define IER_MOSCRCS		17
#define IER_MOSCSELS	16
#define IER_PCKRDY		8
#define IER_LOCKU		6
#define IER_MCKRDY		3
#define IER_LOCKA		1
#define IER_MOSCXTS		0

#define IDR_XT32KERR	21
#define IDR_CFDEV		18
#define IDR_MOSCRCS		17
#define IDR_MOSCSELS	16
#define IDR_PCKRDY		8
#define IDR_LOCKU		6
#define IDR_MCKRDY		3
#define IDR_LOCKA		1
#define IDR_MOSCXTS		0

#define IMR_XT32KERR	21
#define IMR_CFDEV		18
#define IMR_MOSCRCS		17
#define IMR_MOSCSELS	16
#define IMR_PCKRDY		8
#define IMR_LOCKU		6
#define IMR_MCKRDY		3
#define IMR_LOCKA		1
#define IMR_MOSCXTS		0

#define SLPWKER0_PID31	31
#define SLPWKER0_PID30	30
#define SLPWKER0_PID29	29
#define SLPWKER0_PID28	28
#define SLPWKER0_PID27	27
#define SLPWKER0_PID26	26
#define SLPWKER0_PID25	25
#define SLPWKER0_PID24	24
#define SLPWKER0_PID23	23
#define SLPWKER0_PID22	22
#define SLPWKER0_PID21	21
#define SLPWKER0_PID20	20
#define SLPWKER0_PID19	19
#define SLPWKER0_PID18	18
#define SLPWKER0_PID17	17
#define SLPWKER0_PID16	16
#define SLPWKER0_PID15	15
#define SLPWKER0_PID14	14
#define SLPWKER0_PID13	13
#define SLPWKER0_PID12	12
#define SLPWKER0_PID11	11
#define SLPWKER0_PID10	10
#define SLPWKER0_PID9	9
#define SLPWKER0_PID8	8
#define SLPWKER0_PID7	7

#define SLPWKDR0_PID31	31
#define SLPWKDR0_PID30	30
#define SLPWKDR0_PID29	29
#define SLPWKDR0_PID28	28
#define SLPWKDR0_PID27	27
#define SLPWKDR0_PID26	26
#define SLPWKDR0_PID25	25
#define SLPWKDR0_PID24	24
#define SLPWKDR0_PID23	23
#define SLPWKDR0_PID22	22
#define SLPWKDR0_PID21	21
#define SLPWKDR0_PID20	20
#define SLPWKDR0_PID19	19
#define SLPWKDR0_PID18	18
#define SLPWKDR0_PID17	17
#define SLPWKDR0_PID16	16
#define SLPWKDR0_PID15	15
#define SLPWKDR0_PID14	14
#define SLPWKDR0_PID13	13
#define SLPWKDR0_PID12	12
#define SLPWKDR0_PID11	11
#define SLPWKDR0_PID10	10
#define SLPWKDR0_PID9	9
#define SLPWKDR0_PID8	8
#define SLPWKDR0_PID7	7

#define SLPWKSR0_PID31	31
#define SLPWKSR0_PID30	30
#define SLPWKSR0_PID29	29
#define SLPWKSR0_PID28	28
#define SLPWKSR0_PID27	27
#define SLPWKSR0_PID26	26
#define SLPWKSR0_PID25	25
#define SLPWKSR0_PID24	24
#define SLPWKSR0_PID23	23
#define SLPWKSR0_PID22	22
#define SLPWKSR0_PID21	21
#define SLPWKSR0_PID20	20
#define SLPWKSR0_PID19	19
#define SLPWKSR0_PID18	18
#define SLPWKSR0_PID17	17
#define SLPWKSR0_PID16	16
#define SLPWKSR0_PID15	15
#define SLPWKSR0_PID14	14
#define SLPWKSR0_PID13	13
#define SLPWKSR0_PID12	12
#define SLPWKSR0_PID11	11
#define SLPWKSR0_PID10	10
#define SLPWKSR0_PID9	9
#define SLPWKSR0_PID8	8
#define SLPWKSR0_PID7	7

#define SLPWKASR0_PID31	31
#define SLPWKASR0_PID30	30
#define SLPWKASR0_PID29	29
#define SLPWKASR0_PID28	28
#define SLPWKASR0_PID27	27
#define SLPWKASR0_PID26	26
#define SLPWKASR0_PID25	25
#define SLPWKASR0_PID24	24
#define SLPWKASR0_PID23	23
#define SLPWKASR0_PID22	22
#define SLPWKASR0_PID21	21
#define SLPWKASR0_PID20	20
#define SLPWKASR0_PID19	19
#define SLPWKASR0_PID18	18
#define SLPWKASR0_PID17	17
#define SLPWKASR0_PID16	16
#define SLPWKASR0_PID15	15
#define SLPWKASR0_PID14	14
#define SLPWKASR0_PID13	13
#define SLPWKASR0_PID12	12
#define SLPWKASR0_PID11	11
#define SLPWKASR0_PID10	10
#define SLPWKASR0_PID9	9
#define SLPWKASR0_PID8	8
#define SLPWKASR0_PID7	7

#define SLPWKER1_PID62	30
#define SLPWKER1_PID60	28
#define SLPWKER1_PID59	27
#define SLPWKER1_PID58	26
#define SLPWKER1_PID57	25
#define SLPWKER1_PID56	24
#define SLPWKER1_PID53	21
#define SLPWKER1_PID52	20
#define SLPWKER1_PID51	19
#define SLPWKER1_PID50	18
#define SLPWKER1_PID49	17
#define SLPWKER1_PID48	16
#define SLPWKER1_PID47	15
#define SLPWKER1_PID46	14
#define SLPWKER1_PID45	13
#define SLPWKER1_PID44	12
#define SLPWKER1_PID43	11
#define SLPWKER1_PID42	10
#define SLPWKER1_PID41	9
#define SLPWKER1_PID40	8
#define SLPWKER1_PID39	7
#define SLPWKER1_PID37	5
#define SLPWKER1_PID35	3
#define SLPWKER1_PID34	2
#define SLPWKER1_PID33	1
#define SLPWKER1_PID32	0

#define SLPWKDR1_PID62	30
#define SLPWKDR1_PID60	28
#define SLPWKDR1_PID59	27
#define SLPWKDR1_PID58	26
#define SLPWKDR1_PID57	25
#define SLPWKDR1_PID56	24
#define SLPWKDR1_PID53	21
#define SLPWKDR1_PID52	20
#define SLPWKDR1_PID51	19
#define SLPWKDR1_PID50	18
#define SLPWKDR1_PID49	17
#define SLPWKDR1_PID48	16
#define SLPWKDR1_PID47	15
#define SLPWKDR1_PID46	14
#define SLPWKDR1_PID45	13
#define SLPWKDR1_PID44	12
#define SLPWKDR1_PID43	11
#define SLPWKDR1_PID42	10
#define SLPWKDR1_PID41	9
#define SLPWKDR1_PID40	8
#define SLPWKDR1_PID39	7
#define SLPWKDR1_PID37	5
#define SLPWKDR1_PID35	3
#define SLPWKDR1_PID34	2
#define SLPWKDR1_PID33	1
#define SLPWKDR1_PID32	0

#define SLPWKSR1_PID62	30
#define SLPWKSR1_PID60	28
#define SLPWKSR1_PID59	27
#define SLPWKSR1_PID58	26
#define SLPWKSR1_PID57	25
#define SLPWKSR1_PID56	24
#define SLPWKSR1_PID53	21
#define SLPWKSR1_PID52	20
#define SLPWKSR1_PID51	19
#define SLPWKSR1_PID50	18
#define SLPWKSR1_PID49	17
#define SLPWKSR1_PID48	16
#define SLPWKSR1_PID47	15
#define SLPWKSR1_PID46	14
#define SLPWKSR1_PID45	13
#define SLPWKSR1_PID44	12
#define SLPWKSR1_PID43	11
#define SLPWKSR1_PID42	10
#define SLPWKSR1_PID41	9
#define SLPWKSR1_PID40	8
#define SLPWKSR1_PID39	7
#define SLPWKSR1_PID37	5
#define SLPWKSR1_PID35	3
#define SLPWKSR1_PID34	2
#define SLPWKSR1_PID33	1
#define SLPWKSR1_PID32	0

#define SLPWKASR1_PID62	30
#define SLPWKASR1_PID60	28
#define SLPWKASR1_PID59	27
#define SLPWKASR1_PID58	26
#define SLPWKASR1_PID57	25
#define SLPWKASR1_PID56	24
#define SLPWKASR1_PID53	21
#define SLPWKASR1_PID52	20
#define SLPWKASR1_PID51	19
#define SLPWKASR1_PID50	18
#define SLPWKASR1_PID49	17
#define SLPWKASR1_PID48	16
#define SLPWKASR1_PID47	15
#define SLPWKASR1_PID46	14
#define SLPWKASR1_PID45	13
#define SLPWKASR1_PID44	12
#define SLPWKASR1_PID43	11
#define SLPWKASR1_PID42	10
#define SLPWKASR1_PID41	9
#define SLPWKASR1_PID40	8
#define SLPWKASR1_PID39	7
#define SLPWKASR1_PID37	5
#define SLPWKASR1_PID35	3
#define SLPWKASR1_PID34	2
#define SLPWKASR1_PID33	1
#define SLPWKASR1_PID32	0

#define SLPWKAIPR_AIP	0


/* local/static prototypes */
static void init_clk_main(void);
static void init_clk_plla(void);
static void init_clk_upll(void);
static void init_clk_master(void);
static void init_clk_pck(void);
static void init_clk_usb(void);


/* global functions */
int pmc_init(void){
	/* disable interrupts */
	mreg_w(IDR, 0xffffffff);

	/* configure clocks */
	init_clk_main();
	init_clk_plla();
	init_clk_upll();
	init_clk_master();
	init_clk_pck();
	init_clk_usb();

	return E_OK;
}

int pmc_per_enable(unsigned int pid, unsigned int src, unsigned int div){
	/* check inputs */
	// only PID 69 and 70 (I2SC0, I2SC1) support src and div
	if(pid != 69 && pid != 70 && src != 0 && div != 0)
		return_errno(E_INVAL);

	// values too large
	if((pid & ~(0x7f)) || (src & ~(0x7)) || (div & ~(0xff)))
		return_errno(E_INVAL);

	/* configure clock */
	mreg_w(PCR,
		  (0x1 << PCR_CMD)
		| (pid << PCR_PID)
		| (0x1 << PCR_EN)
		| (0x1 << PCR_GCLKEN)
		| (src << PCR_GCLKCSS)
		| (div << PCR_GCLKDIV)
	);

	return E_OK;
}

int pmc_per_disable(unsigned int pid){
	if(pid & ~(0x7f))
		return_errno(E_INVAL);

	mreg_w(PCR,
		  (0x1 << PCR_CMD)
		| (pid << PCR_PID)
	);

	return E_OK;
}


/* local functions */
static void init_clk_main(void){
	// enable oscillators
	mreg_w(MOR, mreg_r(MOR)
		| (CONFIG_PMC_MAINCK_CRYSTAL_EN << MOR_MOSCXTEN)
#ifdef CONFIG_PMC_MAINCK_SRC_CRYSTAL
		| (CONFIG_PMC_MAINCK_CRYSTAL_STARTUP << MOR_MOSCXTST)
#endif
		| (CONFIG_PMC_MAINCK_RC_EN << MOR_MOSCRCEN)
		| (0x37 << MOR_KEY)
	);

#ifdef CONFIG_PMC_MAINCK_SRC_RC
	while(!mreg_bits(SR, SR_MOSCRCS, 0x1));
#endif

#ifdef CONFIG_PMC_MAINCK_SRC_CRYSTAL
	while(!mreg_bits(SR, SR_MOSCXTS, 0x1));
#endif

	// configure oscillators
	mreg_w(MOR,
		  (1 << MOR_XT32KFME)
		| (1 << MOR_CFDEN)
		| (0 << MOR_WAITMODE)
		| (CONFIG_PMC_MAINCK_SRC << MOR_MOSCSEL)
		| (CONFIG_PMC_MAINCK_CRYSTAL_EN << MOR_MOSCXTEN)
#ifdef CONFIG_PMC_MAINCK_SRC_CRYSTAL
		| (CONFIG_PMC_MAINCK_CRYSTAL_STARTUP << MOR_MOSCXTST)
#endif
		| (0 << MOR_MOSCXTBY)
		| (CONFIG_PMC_MAINCK_RC_EN << MOR_MOSCRCEN)
		| (CONFIG_PMC_MAINCK_RC_FREQ << MOR_MOSCRCF)
		| (0x37 << MOR_KEY)
	);

	while(!mreg_bits(SR, SR_MOSCSELS, 0x1));
}

static void init_clk_plla(void){
	mreg_w(PLLAR,
		  (1 << PLLAR_ONE)
		| (0 << PLLAR_PLLACOUNT)
#ifdef CONFIG_PMC_PLLA_EN
		| (CONFIG_PMC_PLLA_MUL << PLLAR_MULA)
		| (CONFIG_PMC_PLLA_DIV << PLLAR_DIVA)
#else
		| (0 << PLLAR_MULA)
		| (0 << PLLAR_DIVA)
#endif
	);

#ifdef CONFIG_PMC_PLLA_EN
	while(!mreg_bits(SR, SR_LOCKA, 0x1));
#endif
}

static void init_clk_upll(void){
#ifdef CONFIG_PMC_UPLL_EN
	// NOTE UCKR has a reset value with undocumented bits set
	mreg_w(UCKR,
		  (CONFIG_PMC_UPLL_EN << UCKR_UPLLEN)
		| (0 << UCKR_UPLLCOUNT)
	);

	while(!mreg_bits(SR, SR_LOCKU, 0x1));

	mreg_w(MCKR, (mreg_r(MCKR) & ~(0x1 << MCKR_UPLLDIV2)) | (CONFIG_PMC_UPLL_DIV << MCKR_UPLLDIV2));
#else
	mreg_w(UCKR, 0x0);
#endif // CONFIG_PMC_UPLL_EN
}

static void init_clk_master(void){
	/* set prescaler and divider in case UPLLDIV or PLLA are the clock source */
#if defined(CONFIG_PMC_HCLK_SRC_UPLLCKDIV) || defined(CONFIG_PMC_HCLK_SRC_PLLACK)
	mreg_w(MCKR, mreg_r(MCKR) | (CONFIG_PMC_HCLK_PRES << MCKR_PRES));
	while(!mreg_bits(SR, SR_MCKRDY, 0x1));

	mreg_w(MCKR, mreg_r(MCKR) | (CONFIG_PMC_HCLK_DIV << MCKR_MDIV));
	while(!mreg_bits(SR, SR_MCKRDY, 0x1));
#endif // UPLLDIV || PLLA

	/* set clock source */
	mreg_w(MCKR, (mreg_r(MCKR) & ~(0x3 << MCKR_CSS)) | (CONFIG_PMC_HCLK_SRC << MCKR_CSS));
	while(!mreg_bits(SR, SR_MCKRDY, 0x1));

	/* set prescaler and divider in case SLCK or MAINCK are the clock source */
#if defined(CONFIG_PMC_HCLK_SRC_SLCK) || defined(CONFIG_PMC_HCLK_SRC_MAINCK)
	mreg_w(MCKR, mreg_r(MCKR)
		| (CONFIG_PMC_HCLK_PRES << MCKR_PRES)
		| (CONFIG_PMC_HCLK_DIV << MCKR_MDIV)
	);

	while(!mreg_bits(SR, SR_MCKRDY, 0x1));
#endif // SLCK || MAINCK
}

static void init_clk_pck(void){
#ifdef CONFIG_PMC_PCKx_EN
	unsigned int i;
	uint32_t *pck;


	for(i=0; i<8; i++){
		pck = (uint32_t*)PCK(i);

		mreg_w(SCDR, 0x1 << (SCDR_PCK0 + i));
		*pck = (CONFIG_PMC_PCKx_SRC << PCKx_CSS) | ((CONFIG_PMC_PCKx_PRES - 1) << PCKx_PRES);
		mreg_w(SCER, 0x1 << (SCER_PCK0 + i));

		while(!mreg_bits(SR, SR_PCKRDY + i, 0x1));
	}
#endif // CONFIG_PMC_PCKx_EN
}

static void init_clk_usb(void){
#ifdef CONFIG_PMC_USB_EN
	mreg_w(USB,
		  (CONFIG_PMC_USB_SRC << USB_USBS)
		| ((CONFIG_PMC_USB_DIV - 1) << USB_USBDIV)
	);

	mreg_w(SCER, (0x1 << SCER_USBCLK));
#else
	mreg_w(SCDR, (0x1 << SCDR_USBCLK));
#endif // CONFIG_PMC_USB_EN
}
