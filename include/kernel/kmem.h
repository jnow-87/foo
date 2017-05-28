#ifndef KERNEL_MEM_H
#define KERNEL_MEM_H


#include <sys/types.h>


/* prototypes */
void *kmalloc(size_t n);
void kfree(void *addr);

void *addr_virt_to_phys(void *va);
void *addr_phys_to_virt(void *pa);


#endif // KERNEL_MEM_H
