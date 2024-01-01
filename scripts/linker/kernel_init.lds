/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/devicetree.h>


. = ALIGN(DEVTREE_ARCH_ADDR_WIDTH / 8);

/* init calls */
__platform_init0_base_cores_all = .;
*(.platform_init_stage0_cores_all)

__platform_init0_base_cores_first = .;
*(.platform_init_stage0_cores_first)

__platform_init1_base_cores_all = .;
*(.platform_init_stage1_cores_all)

__platform_init1_base_cores_first = .;
*(.platform_init_stage1_cores_first)

__kernel_init0_base = .;
*(.kernel_init_stage0*)

__kernel_init1_base = .;
*(.kernel_init_stage1*)

__kernel_init2_base = .;
*(.kernel_init_stage2*)

__init_end = .;

/* driver calls */
__driver_base = .;
*(.driver)
__driver_end = .;
