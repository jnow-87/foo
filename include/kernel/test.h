/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_TEST_H
#define KERNEL_TEST_H


#include <config/config.h>


/* macros */
#ifndef CONFIG_KERNEL_TEST
# define ktest()
#endif // CONFIG_KERNEL_TEST


/* prototypes */
#ifdef CONFIG_KERNEL_TEST
void ktest(void);
#endif // CONFIG_KERNEL_TEST


#endif // KERNEL_TEST_H
