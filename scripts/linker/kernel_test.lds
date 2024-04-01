/**
 * Copyright (C) 2021 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/devicetree.h>


. = ALIGN(DEVTREE_ARCH_ADDR_WIDTH / 8);

__start_tests_kernel = .;
*(tests_kernel)

__stop_tests_kernel = .;
