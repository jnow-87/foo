#include <arch/core.h>
#include <kernel/kmutex.h>
#include <kernel/kprintf.h>


/* static variables */
static kmutex_t kernel_mutex = NESTED_KMUTEX_INITIALISER();


/* global functions */
void kernel_lock(){
	mutex_lock_nested(&kernel_mutex.m);
	DEBUG("[%d] \033[31mlock kernel\033[0m\n", PIR);
}

void kernel_unlock(){
	DEBUG("[%d] \033[32munlock kernel\033[0m\n", PIR);
	mutex_unlock_nested(&kernel_mutex.m);
}
