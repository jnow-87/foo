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
#include <sys/string.h>
#include <sys/uart.h>
#include <sys/vram.h>


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


/* global variables */
unsigned int x86_hw_op_active_tid = 0;


/* global functions */
void x86_hw_int_trigger(int_num_t num, void *payload){
	x86_hw_op_t op;


	op.num = HWO_INT_TRIGGER;
	op.int_ctrl.num = num;
	op.int_ctrl.payload = payload;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);
}

void x86_hw_int_return(int_num_t num, x86_priv_t to, unsigned int tid){
	x86_hw_op_t op;


	op.num = HWO_INT_RETURN;
	op.int_return.num = num;
	op.int_return.to = to;
	op.int_return.tid = tid;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);
}

void x86_hw_int_set(bool en){
	x86_hw_op_t op;


	op.num = HWO_INT_SET;
	op.int_ctrl.en = en;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);
}

void x86_hw_syscall_return(void){
	x86_hw_op_t op;


	op.num = HWO_SYSCALL_RETURN;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);
}

void x86_hw_uart_cfg(char const *path, int rx_int, uart_cfg_t *cfg){
	x86_hw_op_t op;


	if(strlen(path) > 63)
		LNX_EEXIT("uart path \"%s\" too long\n", path);

	op.num = HWO_UART_CFG;
	op.uart.int_num = rx_int;
	op.uart.cfg = *cfg;

	strcpy(op.uart.path, path);

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);
}

void x86_hw_display_cfg(vram_cfg_t *cfg, uint8_t scale, int shm_id){
	x86_hw_op_t op;


	op.num = HWO_DISPLAY_CFG;
	op.display.cfg = *cfg;
	op.display.shm_id = shm_id;
	op.display.scale = scale;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);
}

void x86_hw_setup_complete(int shm_id){
	x86_hw_op_t op;


	op.num = HWO_SETUP;
	op.setup.shm_id = shm_id;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);
}

void x86_hw_usignal_send(void *arg){
	x86_hw_op_t op;


	if(arg == 0x0)
		LNX_EEXIT("no signal arguments set\n");

	op.num = HWO_SIGNAL;
	op.signal.arg = arg;

	x86_hw_op_write(&op);
	x86_hw_op_write_writeback(&op);
}

void x86_hw_op_write(x86_hw_op_t *op){
	static unsigned int seq_num = 0;
	int ack = 0;


	op->src = HW_OP_SRC;
	op->tid = x86_hw_op_active_tid;

	lnx_kill(lnx_getppid(), CONFIG_X86EMU_HW_SIG);

	while(!ack){
		lnx_read_fix(CONFIG_X86EMU_HW_PIPE_RD, &op->seq, sizeof(op->seq));
		CHECK_SEQ_NUM(op->seq, seq_num++);

		lnx_write(CONFIG_X86EMU_HW_PIPE_WR, op, sizeof(*op));
		lnx_read(CONFIG_X86EMU_HW_PIPE_RD, &ack, sizeof(ack));
	}
}

void x86_hw_op_write_writeback(x86_hw_op_t *op){
	unsigned int seq_num = op->seq;


	lnx_read_fix(CONFIG_X86EMU_HW_PIPE_RD, op, sizeof(*op));
	CHECK_SEQ_NUM(op->seq, seq_num);

	lnx_write(CONFIG_X86EMU_HW_PIPE_WR, &op->seq, sizeof(op->seq));

	if(op->retval != 0)
		LNX_EEXIT("[%u] hardware-op %d returned failure %d\n", op->seq, op->num, op->retval);
}

void x86_hw_op_read(x86_hw_op_t *op){
	static unsigned int seq_num = 0;


	lnx_write(CONFIG_X86EMU_HW_PIPE_WR, &seq_num, sizeof(seq_num));

	lnx_read_fix(CONFIG_X86EMU_HW_PIPE_RD, op, sizeof(*op));
	CHECK_SEQ_NUM(op->seq, seq_num++);
}

void x86_hw_op_read_writeback(x86_hw_op_t *op){
	unsigned int seq_num;


	lnx_write(CONFIG_X86EMU_HW_PIPE_WR, op, sizeof(*op));

	lnx_read_fix(CONFIG_X86EMU_HW_PIPE_RD, &seq_num, sizeof(seq_num));
	CHECK_SEQ_NUM(seq_num, op->seq);
}
