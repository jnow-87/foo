/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <arch/x86/linux.h>
#include <arch/x86/hardware.h>
#include <sys/compiler.h>


/* macros */
#if defined(BUILD_KERNEL)
# define HW_OP_SRC	HWS_KERNEL
#elif defined(BUILD_LIBSYS)
# define HW_OP_SRC	HWS_USER
#else
CPP_ASSERT(invalid build config)
#endif // BUILD_KERNEL

#define CHECK_SEQ_NUM(num, ref){ \
	typeof(num) _num = num; \
	typeof(ref) _ref = ref; \
	\
	\
	if(_num != _ref) \
		LNX_EEXIT("sequence number mismatch: %u expected %u\n", _num, _ref); \
}


/* global functions */
void x86_hw_op_write(x86_hw_op_t *op){
	static unsigned int seq_num = 0;
	int ack;


	op->src = HW_OP_SRC;

	lnx_kill(lnx_getppid(), CONFIG_TEST_INT_DATA_SIG);

	ack = 0;

	while(!ack){
		lnx_read_fix(CONFIG_TEST_INT_DATA_PIPE_RD, &op->seq, sizeof(op->seq));
		CHECK_SEQ_NUM(op->seq, seq_num++);

		lnx_write(CONFIG_TEST_INT_DATA_PIPE_WR, op, sizeof(*op));
		lnx_read(CONFIG_TEST_INT_DATA_PIPE_RD, &ack, sizeof(ack));
	}
}

void x86_hw_op_write_writeback(x86_hw_op_t *op){
	unsigned int seq_num;


	seq_num = op->seq;

	lnx_read_fix(CONFIG_TEST_INT_DATA_PIPE_RD, op, sizeof(*op));
	CHECK_SEQ_NUM(op->seq, seq_num);

	lnx_write(CONFIG_TEST_INT_DATA_PIPE_WR, &op->seq, sizeof(op->seq));

	if(op->retval != 0)
		LNX_EEXIT("[%u] hardware-op %d returned failure %d\n", op->seq, op->num, op->retval);
}

void x86_hw_op_read(x86_hw_op_t *op){
	static unsigned int seq_num = 0;


	lnx_write(CONFIG_TEST_INT_DATA_PIPE_WR, &seq_num, sizeof(seq_num));

	lnx_read_fix(CONFIG_TEST_INT_DATA_PIPE_RD, op, sizeof(*op));
	CHECK_SEQ_NUM(op->seq, seq_num++);
}

void x86_hw_op_read_writeback(x86_hw_op_t *op){
	unsigned int seq_num;


	lnx_write(CONFIG_TEST_INT_DATA_PIPE_WR, op, sizeof(*op));

	lnx_read_fix(CONFIG_TEST_INT_DATA_PIPE_RD, &seq_num, sizeof(seq_num));
	CHECK_SEQ_NUM(seq_num, op->seq);
}
