/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARM_BOARD_TYPES_H
#define ARM_BOARD_TYPES_H


/* types */
typedef struct{
	int (*board_init)(void);

	int (*peripheral_enable)(unsigned int id, unsigned int clk_src, unsigned int clk_div);
	int (*peripheral_disable)(unsigned int id);
} arm_board_callbacks_t;


#endif // ARM_BOARD_TYPES_H
