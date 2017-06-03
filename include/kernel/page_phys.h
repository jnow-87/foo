#ifndef KERNEL_PAGE_PHYS_H
#define KERNEL_PAGE_PHYS_H


#include <kernel/umem.h>


/* macros */
#define page_alloc(process, size)	(umalloc(size))
#define page_free(process, page)	(ufree(page))


/* types */
typedef void *page_t;


#endif // KERNEL_PAGE_PHYS_H
