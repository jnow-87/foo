#ifndef KERNEL_PAGE_VIRT_H
#define KERNEL_PAGE_VIRT_H


#include <kernel/process.h>


/* macros */
#define PAGESIZE_BYTES(psize)	((0x1 << (2 * psize)) * 4096)


/* types */
typedef enum{
	PAGE_R = 0x1,
	PAGE_W = 0x2,
	PAGE_X = 0x4
} page_perm_t;

typedef enum{
	PAGE_KERNEL = 0,
	PAGE_USER
} page_priv_t;

typedef enum{
	PAGE_COHERENT = 0x1,
	PAGE_NOREORDER = 0x2,
	PAGE_NOCACHE = 0x4,
	PAGE_INVAL_PROT = 0x8,
	PAGE_WRITE_THROUGH = 0x10
} page_flags_t;

typedef enum{
	PAGESIZE_4k = 0x0,
	PAGESIZE_16k,
	PAGESIZE_64k,
	PAGESIZE_256k,
	PAGESIZE_1M,
	PAGESIZE_4M,
	PAGESIZE_16M,
	PAGESIZE_64M,
	PAGESIZE_256M,
	PAGESIZE_1G
} page_size_t;

typedef struct page_t{
	/* mapping */
	page_size_t psize;							// page size
	void *phys_addr,							// physical address
		 *virt_addr;							// virtual address

	/* flags */
	page_flags_t flags;							// other flags
	page_priv_t priv;							// privilege level
	page_perm_t perm;							// permissions

	/* index */
	unsigned int idx;							// index in page table/TLB

	/* list handling */
	struct page_t *prev,
				  *next;
} page_t;


/* prototypes */
page_t *page_alloc(process_t *this_p, page_size_t psize);
void page_free(process_t *this_p, page_t *page);


#endif // KERNEL_PAGE_VIRT_H
