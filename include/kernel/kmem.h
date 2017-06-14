#ifndef KERNEL_MEM_H
#define KERNEL_MEM_H


#include <config/config.h>
#include <sys/types.h>


/* prototypes */
void *kmalloc(size_t n);
void kfree(void *addr);

#ifdef CONFIG_KERNEL_VIRT_MEM

void *addr_virt_to_phys(void *va);
void *addr_phys_to_virt(void *pa);

#endif // CONFIG_KERNEL_VIRT_MEM


#endif // KERNEL_MEM_H
