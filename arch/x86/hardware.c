/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/x86/linux.h>
#include <arch/x86/hardware.h>
#include <kernel/interrupt.h>
#include <sys/compiler.h>


/* macros */
#if defined(BUILD_KERNEL)
# define HW_OP_SRC	PRIV_KERNEL
#elif defined(BUILD_LIBBRICK)
# define HW_OP_SRC	PRIV_USER
#else
STATIC_ASSERT(!"invalid build config");
#endif // BUILD_KERNEL

#define CHECK_SEQ_NUM(num, ref){ \
	typeof(num) _num = num; \
	typeof(ref) _ref = ref; \
	\
	\
	if(_num != _ref) \
		LNX_EEXIT("sequence number mismatch: %u expected %u\n", _num, _ref); \
}

static char const *ops_name[] = {
	"exit",
	"int_trigger",
	"int_return",
	"int_set",
	"int_state",
	"syscall_return",
	"copy_from_user",
	"copy_to_user",
	"uart_config",
	"display_config",
	"setup",
	"invalid",
};


/* global variables */
unsigned int x86_hw_op_active_tid = 0;


/* global functions */
void x86_hw_op_write(x86_hw_op_t *op){
	static unsigned int seq_num = 0;
	unsigned int ack = 0;


	op->src = HW_OP_SRC;
	op->tid = x86_hw_op_active_tid;

//	if(seq_num > 30)
//		lnx_exit(123);
//
	LNX_DEBUG("trigger event %u %s\n", seq_num, ops_name[op->num]);
	lnx_kill(lnx_getppid(), CONFIG_TEST_INT_HW_SIG);

	while(!ack){
		lnx_read_fix(CONFIG_TEST_INT_HW_PIPE_RD, &op->seq, sizeof(op->seq));
		CHECK_SEQ_NUM(op->seq, seq_num++);

		lnx_write(CONFIG_TEST_INT_HW_PIPE_WR, op, sizeof(*op));
		lnx_read(CONFIG_TEST_INT_HW_PIPE_RD, &ack, sizeof(ack));

		if(!ack){
			LNX_DEBUG("nack\n");
		}
		else
			CHECK_SEQ_NUM(ack, op->seq + 1);
	}
}

void x86_hw_op_write_writeback(x86_hw_op_t *op){
	unsigned int seq_num = op->seq;


	lnx_read_fix(CONFIG_TEST_INT_HW_PIPE_RD, op, sizeof(*op));
	CHECK_SEQ_NUM(op->seq, seq_num);

	lnx_write(CONFIG_TEST_INT_HW_PIPE_WR, &op->seq, sizeof(op->seq));

	if(op->retval != 0)
		LNX_EEXIT("[%u] hardware-op %d returned failure %d\n", op->seq, op->num, op->retval);
}

void x86_hw_op_read(x86_hw_op_t *op){
	static unsigned int seq_num = 0;


	lnx_write(CONFIG_TEST_INT_HW_PIPE_WR, &seq_num, sizeof(seq_num));

	lnx_read_fix(CONFIG_TEST_INT_HW_PIPE_RD, op, sizeof(*op));
	CHECK_SEQ_NUM(op->seq, seq_num++);
}

void x86_hw_op_read_writeback(x86_hw_op_t *op){
	unsigned int seq_num;


	lnx_write(CONFIG_TEST_INT_HW_PIPE_WR, op, sizeof(*op));

	lnx_read_fix(CONFIG_TEST_INT_HW_PIPE_RD, &seq_num, sizeof(seq_num));
	CHECK_SEQ_NUM(seq_num, op->seq);
}

void x86_hw_int_trigger(int_num_t num, void *payload){
	x86_hw_op_t op;


	op.num = HWO_INT_TRIGGER;
	op.int_ctrl.num = num;
	op.int_ctrl.payload = payload;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);
}
