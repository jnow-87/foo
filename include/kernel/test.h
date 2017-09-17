#ifndef KERNEL_TEST_H
#define KERNEL_TEST_H


#include <config/config.h>


/* prototypes */
#ifdef CONFIG_KERNEL_TEST
void kernel_test();
#else
#define kernel_test()
#endif // CONFIG_KERNEL_TEST


#endif // KERNEL_TEST_H
