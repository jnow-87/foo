/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



__core_init0_base = .;
*(.core_init_stage0)

__platform_init0_base = .;
*(.platform_init_stage0)

__platform_init1_base = .;
*(.platform_init_stage1)

__kernel_init0_base = .;
*(.kernel_init_stage0)

__kernel_init1_base = .;
*(.kernel_init_stage1)

__kernel_init2_base = .;
*(.kernel_init_stage2)

__driver_init0_base = .;
*(.driver_init_stage0)

__init_end = .;
