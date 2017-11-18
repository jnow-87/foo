#include <config/config.h>
#include <arch/interrupt.h>
#include <kernel/kprintf.h>

#ifdef CONFIG_KERNEL_SMP
#include <sys/mutex.h>
#endif // CONFIG_KERNEL_SMP


/* static variables */
static int_type_t imask;

#ifdef CONFIG_KERNEL_SMP
static mutex_t kmutex = NESTED_MUTEX_INITIALISER();
#endif // CONFIG_KERNEL_SMP


/* global functions */
void klock(){
#ifdef CONFIG_KERNEL_SMP
	mutex_lock_nested(&kmutex);
#endif // CONFIG_KERNEL_SMP

	imask = int_enabled();
	int_enable(INT_NONE);

	DEBUG("\033[31mlock kernel\033[0m\n");
}

void kunlock(){
	DEBUG("\033[32munlock kernel\033[0m\n");

	int_enable(imask);

#ifdef CONFIG_KERNEL_SMP
	mutex_unlock_nested(&kmutex);
#endif // CONFIG_KERNEL_SMP
}
