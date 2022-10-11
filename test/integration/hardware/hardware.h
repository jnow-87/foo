/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef TEST_INT_HARDWARE_H
#define TEST_INT_HARDWARE_H


#include <arch/x86/hardware.h>
#include <sys/types.h>
#include <sys/uart.h>
#include <sys/vram.h>
#include <brickos/child.h>


/* types */
typedef struct{
	size_t event_ack,
		   event_nack,
		   int_ack,
		   int_nack;
} hw_stats_t;

typedef struct{
	x86_priv_t privilege;
	unsigned int tid;

	bool int_enabled;
	size_t ints_active;

	bool locked;

	hw_stats_t stats;
} hw_state_t;


/* prototypes */
// hardware state
void hw_state_lock(void);
void hw_state_unlock(void);

void hw_state_wait(void);

void hw_state_print(void);

// hardware ops
void hw_op_write(x86_hw_op_t *op, child_t *tgt);
void hw_op_write_sig(x86_hw_op_t *op, child_t *tgt, int sig);
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
void hw_int_request(int num, void *payload, x86_priv_t src, unsigned int tid);
void hw_int_return(int num, x86_priv_t target, unsigned int tid);

// interrupt timers
void hw_timer(void);
void hw_sched_timer(void);

// uart
int uart_init(void);
void uart_cleanup(void);

void uart_poll(void);
int uart_configure(char const *path, int int_num, uart_cfg_t *cfg);

// display
int display_init(void);
void display_cleanup(void);

void display_poll(void);
int display_configure(int shm_id, uint8_t scale, vram_cfg_t *cfg);


/* external variables */
extern hw_state_t hw_state;


#endif // TEST_INT_HARDWARE_H
