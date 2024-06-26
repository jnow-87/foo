/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86_HARDWARE_H
#define X86_HARDWARE_H


#include <kernel/interrupt.h>
#include <sys/devicetree.h>
#include <sys/types.h>
#include <sys/uart.h>
#include <sys/vram.h>


/* macros */
#define X86_INT_PRIOS	2

#define X86_INT_NAME(num) \
	(((char*[]){ \
		"timer", \
		"scheduler", \
		"syscall", \
		"uart0", \
		"uart1", \
	})[num])

#define X86_PRIV_NAME(priv) \
	(((char*[]){ \
		"kernel", \
		"user", \
		"hardware", \
	})[priv])


/* types */
typedef enum{
	INT_TIMER = 0,
	INT_SYSCALL,
	INT_UART0,
	INT_UART1,
	INT_NUM_HWINTS
} x86_int_num_t;

STATIC_ASSERT(DEVTREE_ARCH_NUM_INTS >= INT_NUM_HWINTS);

typedef enum{
	PRIV_KERNEL = 0,
	PRIV_USER,
	PRIV_HARDWARE
} x86_priv_t;

typedef enum{
	HWO_INT_TRIGGER = 0,
	HWO_INT_RETURN,
	HWO_INT_SET,
	HWO_SYSCALL_RETURN,
	HWO_COPY_FROM_USER,
	HWO_COPY_TO_USER,
	HWO_UART_CFG,
	HWO_DISPLAY_CFG,
	HWO_SETUP,
	HWO_SIGNAL,
	HWO_NOPS
} x86_hw_op_num_t;

typedef struct{
	unsigned int num;	/**< cf. x86_hw_op_num_t */
	unsigned int src;	/**< cf. x86_priv_t */
	unsigned int seq;
	unsigned int tid;

	int retval;

	union{
		struct{
			int retval;
		} exit;

		struct{
			int num;
			bool en;
			void *payload;
		} int_ctrl;

		struct{
			int num;
			int to;		/**< cf. x86_priv_t */
			unsigned int tid;
		} int_return;

		struct{
			void *addr;
			ssize_t n;
		} copy;

		struct{
			char path[64];
			int int_num;
			uart_cfg_t cfg;
		} uart;

		struct{
			int shm_id;
			uint8_t scale;
			vram_cfg_t cfg;
		} display;

		struct{
			int shm_id;
		} setup;

		struct{
			void *arg;	// parameter  according to thread_entry_t
		} signal;
	};
} x86_hw_op_t;


/* prototypes */
void x86_hw_int_trigger(int_num_t num, void *payload);
void x86_hw_int_return(int_num_t num, x86_priv_t to, unsigned int tid);
void x86_hw_int_set(bool en);
void x86_hw_syscall_return(void);
void x86_hw_uart_cfg(char const *path, int rx_int, uart_cfg_t *cfg);
void x86_hw_display_cfg(vram_cfg_t *cfg, uint8_t scale, int shm_id);
void x86_hw_setup_complete(int shm_id);
void x86_hw_usignal_send(void *arg);

void x86_hw_op_write(x86_hw_op_t *op);
void x86_hw_op_write_writeback(x86_hw_op_t *op);

void x86_hw_op_read(x86_hw_op_t *op);
void x86_hw_op_read_writeback(x86_hw_op_t *op);


/* external variables */
extern unsigned int x86_hw_op_active_tid;


#endif // X86_HARDWARE_H
