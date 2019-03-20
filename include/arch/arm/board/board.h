/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARM_BOARD_H
#define ARM_BOARD_H


#include <arch/arch.h>
#include <sys/errno.h>


/* macros */
#define arm_board_call(p, err_ret) \
	(arm_board_cbs.p == 0) ? (err_ret) : arm_board_cbs.p


#define board_init()						(arm_board_call(board_init, -E_NOIMP)())
#define per_clk_enable(id, src, pres, div)	(arm_board_call(per_clk_enable, -E_NOIMP)(id, src, pres, div))
#define per_clk_disable(id)					(arm_board_call(per_clk_disable, -E_NOIMP)(id))


#endif // ARM_BOARD_H
