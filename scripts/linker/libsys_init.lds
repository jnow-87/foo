/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/devicetree.h>


. = ALIGN(DEVTREE_ARCH_ADDR_WIDTH / 8);

__lib_init0_base = .;
*(.lib_init_stage0)
__lib_init0_end = .;

__lib_init1_base = .;
*(.lib_init_stage1)
__lib_init1_end = .;
