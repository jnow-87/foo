/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef TEST_INT_HARDWARE_H
#define TEST_INT_HARDWARE_H


#include <stdbool.h>
#include <include/arch/x86/hardware.h>
#include <brickos/child.h>


/* types */
typedef struct{
	x86_hw_op_src_t priviledge;
	bool int_enabled;
	bool syscall_pending;
} hw_state_t;


/* prototypes */
// hardware state
void hw_state_lock(void);
void hw_state_unlock(void);

void hw_state_wait(void);

// hardware ops
void hw_op_write(x86_hw_op_t *op, child_t *tgt);
void hw_op_write_writeback(x86_hw_op_t *op, child_t *tgt);
void hw_op_read(x86_hw_op_t *op, child_t *src);
void hw_op_read_ack(child_t *src, int ack);
void hw_op_read_writeback(x86_hw_op_t *op, child_t *src);

// hardware event handling
void hw_event_process(void);
void hw_event_enqueue(child_t *src);
child_t *hw_event_dequeue(void);

// interrupt handling
void hw_int_process(void);
void hw_int_request(int num, void *data, x86_hw_op_src_t src);
void hw_int_return(x86_hw_op_src_t target);

// interrupt timer
void hw_timer(void);


/* external variables */
extern hw_state_t hw_state;


#endif // TEST_INT_HARDWARE_H
