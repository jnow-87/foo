#include <config/config.h>
#include <arch/interrupt.h>
#include <kernel/kprintf.h>
#include <sys/types.h>

#ifdef CONFIG_KERNEL_SMP
#include <sys/mutex.h>
#endif // CONFIG_KERNEL_SMP


/* static variables */
static volatile int_type_t imask = INT_NONE;
static volatile uint8_t nest = 0;

#ifdef CONFIG_KERNEL_SMP
static mutex_t kmutex = NESTED_MUTEX_INITIALISER();
#endif // CONFIG_KERNEL_SMP


/* global functions */
void klock(){
#ifdef CONFIG_KERNEL_SMP
	mutex_lock_nested(&kmutex);
#endif // CONFIG_KERNEL_SMP

	if(nest == 0)
		imask = int_enabled();

	int_enable(INT_NONE);
	nest++;

	DEBUG("\033[31mlock kernel\033[0m\n");
}

void kunlock(){
	DEBUG("\033[32munlock kernel\033[0m\n");

	nest--;

	if(nest == 0)
		int_enable(imask);

#ifdef CONFIG_KERNEL_SMP
	mutex_unlock_nested(&kmutex);
#endif // CONFIG_KERNEL_SMP
}
