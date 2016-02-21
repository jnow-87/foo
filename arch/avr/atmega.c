#include <config/config.h>
#include <arch/arch.h>


/* global variables */
// kernel callbacks
#ifdef KERNEL

arch_callbacks_kernel_t arch_cbs_kernel = {
	.page_entry_write = 0x0,
	.page_entry_inval_idx = 0x0,
	.page_entry_inval_va = 0x0,
	.page_entry_search = 0x0,

	.copy_from_user = 0x0,
	.copy_to_user = 0x0,

	.int_enable = 0x0,
	.int_get_mask = 0x0,
	.int_hdlr_register = 0x0,
	.int_hdlr_release = 0x0,

	.ipi_sleep = 0x0,
	.ipi_wake = 0x0,

	.thread_call = 0x0,
	.thread_kill = 0x0,

	.putchar = 0x0,
	.puts = 0x0,
	.getchar = 0x0,
};

#endif // KERNEL

// common callbacks
arch_callbacks_common_t arch_cbs_common = {
	.timebase = 0x0,
	.timebase_to_time = 0x0,

	/* atomics */
	.cas = 0x0,

	/* core */
	.core_id = 0x0,
	.core_sleep = 0x0,
	.core_halt = 0x0,

	/* syscall */
	.syscall = 0x0,
};

// architecture info
arch_info_t arch_info = {
	.core_clock_khz = CONFIG_CORE_CLOCK_HZ / 1000,
	.timebase_clock_khz = 0x0,
};
