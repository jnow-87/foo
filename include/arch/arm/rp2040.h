/**
 * Copyright (C) 2023 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef RP2040_H
#define RP2040_H


#include <arch/arm/v6m.h>
#include <arch/types.h>
#include <kernel/ipi.h>
#include <sys/devtree.h>
#include <sys/types.h>


/* macros */
// memory map
#define CLOCKS_BASE		0x40008000
#define RESETS_BASE		0x4000c000
#define IO_BANK0_BASE	0x40014000
#define PADS_BANK0_BASE	0x4001c000
#define PADS_QSPI_BASE	0x40020000
#define XOSC_BASE		0x40024000
#define PLL_SYS_BASE	0x40028000
#define PLL_USB_BASE	0x4002c000
#define ROSC_BASE		0x40060000
#define SIO_BASE		0xd0000000

// clocks
#define RP2040_PLATFORM_CONFIG		((rp2040_platform_cfg_t const*)devtree_arch_payload("rp2040,platform"))
#define RP2040_SYSTEM_CLOCK_HZ		(RP2040_PLATFORM_CONFIG->system_clock_khz * 1000)
#define RP2040_REF_CLOCK_HZ			(RP2040_PLATFORM_CONFIG->crystal_clock_khz * 1000)
#define RP2040_PERI_CLOCK_HZ		(RP2040_PLATFORM_CONFIG->peri_clock_khz * 1000)


/* types */
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
	RP2040_RST_ADC = 0,
	RP2040_RST_BUSCTRL,
	RP2040_RST_DMA,
	RP2040_RST_I2C0,
	RP2040_RST_I2C1,
	RP2040_RST_IO_BANK0,
	RP2040_RST_IO_QSPI,
	RP2040_RST_JTAG,
	RP2040_RST_PADS_BANK0,
	RP2040_RST_PADS_QSPI,
	RP2040_RST_PIO0,
	RP2040_RST_PIO1,
	RP2040_RST_PLL_SYS,
	RP2040_RST_PLL_USB,
	RP2040_RST_PWM,
	RP2040_RST_RTC,
	RP2040_RST_SPI0,
	RP2040_RST_SPI1,
	RP2040_RST_SYSCFG,
	RP2040_RST_SYSINFO,
	RP2040_RST_TBMAN,
	RP2040_RST_TIMER,
	RP2040_RST_UART0,
	RP2040_RST_UART1,
	RP2040_RST_USBCTRL,
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

void rp2040_resets_release(uint32_t mask);
void rp2040_resets_halt(uint32_t mask);

int rp2040_atomic(atomic_t op, void *param);

void rp2040_iomux_init(void);
void rp2040_clocks_init(void);


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
