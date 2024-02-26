/**
 * Copyright (C) 2017 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/devicetree.h>


. = ALIGN(DEVTREE_ARCH_ADDR_WIDTH / 8);

__kernel_stat_base = .;
*(.kernel_stat)

__kernel_stat_end = .;
