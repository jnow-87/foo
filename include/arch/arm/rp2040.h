/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef RP2040_H
#define RP2040_H


#include <arch/types.h>
#include <arch/arm/v6m.h>
#include <kernel/ipi.h>
#include <sys/devtree.h>
#include <sys/types.h>


/* macros */
// clocks
#define RP2040_PLATFORM_CONFIG		((rp2040_platform_cfg_t const*)devtree_arch_payload("rp2040,platform"))
#define RP2040_SYSTEM_CLOCK_HZ		(RP2040_PLATFORM_CONFIG->system_clock_khz * 1000)
#define RP2040_PERI_CLOCK_HZ		(RP2040_PLATFORM_CONFIG->peri_clock_khz * 1000)


/* types */
typedef enum{
	RP2040_CS_NONE = 0x0,
	RP2040_CS_PLL_SYS_CLKSRC,
	RP2040_CS_PLL_USB_CLKSRC,
	RP2040_CS_ROSC_CLKSRC,
	RP2040_CS_ROSC_CLKSRC_PH,
	RP2040_CS_XOSC_CLKSRC,
	RP2040_CS_CLKSRC_GPIN0,
	RP2040_CS_CLKSRC_GPIN1,
	RP2040_CS_CLK_REF,
	RP2040_CS_CLK_SYS,
	RP2040_CS_CLK_PERI,
	RP2040_CS_CLK_USB,
	RP2040_CS_CLK_ADC,
	RP2040_CS_CLK_RTC,
} rp2040_clk_src_t;

typedef enum{
	RP2040_GPIO_FUNC_XIP = 0,
	RP2040_GPIO_FUNC_SPI,
	RP2040_GPIO_FUNC_UART,
	RP2040_GPIO_FUNC_I2C,
	RP2040_GPIO_FUNC_PWM,
	RP2040_GPIO_FUNC_SIO,
	RP2040_GPIO_FUNC_PIO0,
	RP2040_GPIO_FUNC_PIO1,
	RP2040_GPIO_FUNC_CLK,
	RP2040_GPIO_FUNC_USB,
	RP2040_GPIO_FUNC_RESET = 31,
} rp2040_gpio_func_t;

typedef enum{
	RP2040_RST_ADC = 0x1,
	RP2040_RST_BUSCTRL = 0x2,
	RP2040_RST_DMA = 0x4,
	RP2040_RST_I2C0 = 0x8,
	RP2040_RST_I2C1 = 0x10,
	RP2040_RST_IO_BANK0 = 0x20,
	RP2040_RST_IO_QSPI = 0x40,
	RP2040_RST_JTAG = 0x80,
	RP2040_RST_PADS_BANK0 = 0x100,
	RP2040_RST_PADS_QSPI = 0x200,
	RP2040_RST_PIO0 = 0x400,
	RP2040_RST_PIO1 = 0x800,
	RP2040_RST_PLL_SYS = 0x1000,
	RP2040_RST_PLL_USB = 0x2000,
	RP2040_RST_PWM = 0x4000,
	RP2040_RST_RTC = 0x8000,
	RP2040_RST_SPI0 = 0x10000,
	RP2040_RST_SPI1 = 0x20000,
	RP2040_RST_SYSCFG = 0x40000,
	RP2040_RST_SYSINFO = 0x80000,
	RP2040_RST_TBMAN = 0x100000,
	RP2040_RST_TIMER = 0x200000,
	RP2040_RST_UART0 = 0x400000,
	RP2040_RST_UART1 = 0x800000,
	RP2040_RST_USBCTRL = 0x1000000,
} rp2040_resets_id_t;

typedef struct{
	uint8_t ref_div;		/**< reference clock divider, range = [1, 63] */
	uint8_t post_div[2];	/**< post dividers 1 and 2, range = [1, 7] */
	uint16_t feeback_div;	/**< feedback divider, range = [16, 320] */
} rp2040_pll_cfg_t;

typedef struct{
	uint32_t crystal_clock_khz,		/**< external crystal frequency */
			 system_clock_khz,		/**< system clock, set by the platform implementation */
			 peri_clock_khz;		/**< peripherall clock, set by the platform implementation */

	rp2040_pll_cfg_t pll_sys,
					 pll_usb;

	uint8_t gpio_funcsel[30];		/**< cf. rp2040_gpio_func_t */
	uint8_t gpio_v33;				/**< 3.3V for gpio pins*/
} rp2040_platform_cfg_t;


/* prototypes */
int rp2040_core_id(void);
void rp2040_cores_boot(void);

int rp2040_ipi_int(unsigned int core, bool bcast, ipi_msg_t *msg);
ipi_msg_t *rp2040_ipi_arg(void);

void rp2040_resets_release(rp2040_resets_id_t io);
void rp2040_resets_halt(rp2040_resets_id_t io);

int rp2040_atomic(atomic_t op, void *param);

void rp2040_iomux_init(void);
void rp2040_clocks_init(void);
uint32_t rp2040_clocks_measure(rp2040_clk_src_t src);


/* static variables */
// kernel ops
#ifdef BUILD_KERNEL
static arch_ops_kernel_t const arch_ops_kernel = {
	/* core */
	.core_id = rp2040_core_id,
	.core_sleep = av6m_core_sleep,
	.core_panic = av6m_core_panic,

# ifdef DEVTREE_ARCH_MULTI_CORE
	.cores_boot = rp2040_cores_boot,
# else
	.cores_boot = 0x0,
# endif // DEVTREE_ARCH_MULTI_CORE

	/* virtual memory management */
	.page_entry_write = 0x0,
	.page_entry_inval_idx = 0x0,
	.page_entry_inval_va = 0x0,
	.page_entry_search = 0x0,

	.copy_from_user = 0x0,
	.copy_to_user = 0x0,

	/* interrupts */
	.int_enable = av6m_int_enable,
	.int_enabled = av6m_int_enabled,

# ifdef DEVTREE_ARCH_MULTI_CORE
	.ipi_int = rp2040_ipi_int,
	.ipi_arg = rp2040_ipi_arg,
# else
	.ipi_int = 0x0,
	.ipi_arg = 0x0,
# endif // DEVTREE_ARCH_MULTI_CORE

	/* threading */
	.thread_ctx_init = av6m_thread_ctx_init,

	/* syscall */
	.sc_arg = av6m_sc_arg,
};
#endif // BUILD_KERNEL

// common ops
static arch_ops_common_t const arch_ops_common = {
	/* atomics */
	.atomic = rp2040_atomic,
	.cas = 0x0,
	.atomic_add = 0x0,

	/* syscall */
	.sc = av6m_sc,
};


#endif // RP2040_H
