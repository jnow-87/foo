#ifndef KERNEL_UMEM_H
#define KERNEL_UMEM_H


#include <sys/types.h>


/* prototypes */
void *umalloc(size_t n);
void ufree(void *addr);


#endif // KERNEL_UMEM_H
