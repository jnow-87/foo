/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86_HARDWARE_H
#define X86_HARDWARE_H


#include <kernel/interrupt.h>
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
	INT_SCHED,
	INT_SYSCALL,
	INT_UART0,
	INT_UART1,
	ARCH_NUM_INTS
} x86_int_num_t;

typedef enum{
	PRIV_KERNEL = 0,
	PRIV_USER,
	PRIV_HARDWARE
} x86_priv_t;

typedef enum{
	HWO_EXIT = 0,
	HWO_INT_TRIGGER,
	HWO_INT_RETURN,
	HWO_INT_SET,
	HWO_INT_STATE,
	HWO_SYSCALL_RETURN,
	HWO_COPY_FROM_USER,
	HWO_COPY_TO_USER,
	HWO_UART_CFG,
	HWO_DISPLAY_CFG,
	HWO_SETUP,
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
			int en;
			void *data;
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
	};
} x86_hw_op_t;


/* prototypes */
void x86_hw_op_write(x86_hw_op_t *op);
void x86_hw_op_write_writeback(x86_hw_op_t *op);

void x86_hw_op_read(x86_hw_op_t *op);
void x86_hw_op_read_writeback(x86_hw_op_t *op);

void x86_hw_int_trigger(int_num_t num, void *data);


/* external variables */
extern unsigned int x86_hw_op_active_tid;


#endif // X86_HARDWARE_H
