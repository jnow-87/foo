#ifndef KERNEL_TEST_H
#define KERNEL_TEST_H


#include <config/config.h>


/* prototypes */
#ifdef CONFIG_KERNEL_TEST
void ktest();
#else
#define ktest()
#endif // CONFIG_KERNEL_TEST


#endif // KERNEL_TEST_H
