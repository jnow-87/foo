/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86_HARDWARE_H
#define X86_HARDWARE_H


#include <sys/types.h>


/* macros */
// interrupts
#define NUM_INT		2

#define INT_TIMER	0
#define INT_SYSCALL	1


/* types */
typedef enum{
	HWS_KERNEL = 0,
	HWS_USER,
	HWS_HARDWARE
} x86_hw_op_src_t;

typedef enum{
	HWO_EXIT = 0,
	HWO_INT_TRIGGER,
	HWO_INT_RETURN,
	HWO_INT_SET,
	HWO_INT_STATE,
	HWO_COPY_FROM_USER,
	HWO_COPY_TO_USER,
	HWO_NOPS
} x86_hw_op_num_t;

typedef struct{
	unsigned int num;	/**< cf. x86_hw_op_num_t */
	unsigned int src;	/**< cf. x86_hw_op_src_t */
	unsigned int seq;

	int retval;

	union{
		struct{
			int retval;
		} exit;

		struct{
			int num;
			int en;
			int ret_to;	/**< cf. x86_hw_op_src_t */
			void *data;
		} int_ctrl;

		struct{
			void *addr;
			ssize_t n;
		} copy;
	};
} x86_hw_op_t;


/* prototypes */
void x86_hw_op_write(x86_hw_op_t *op);
void x86_hw_op_write_writeback(x86_hw_op_t *op);

void x86_hw_op_read(x86_hw_op_t *op);
void x86_hw_op_read_writeback(x86_hw_op_t *op);


#endif // X86_HARDWARE_H
