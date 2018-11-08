/*
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_H
#define KERNEL_H


#include <config/config.h>


/* prototypes */
int kinit(void);

#ifdef CONFIG_KERNEL_STAT
void kstat(void);
#else
#define kstat()
#endif // CONFIG_KERNEL_STAT

#ifdef CONFIG_KERNEL_TEST
void ktest(void);
#else
#define ktest()
#endif // CONFIG_KERNEL_TEST


#endif // KERNEL_H
