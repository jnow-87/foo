#ifndef KERNEL_PAGE_H
#define KERNEL_PAGE_H


#include <sys/math.h>
#include <sys/compiler.h>


/* macros */
#define PAGESIZE_BYTES(psize)		(powl(4, (psize) + 1) * 1024)


/* types */
typedef enum __packed{
	PAGE_R = 0x1,
	PAGE_W = 0x2,
	PAGE_X = 0x4
} page_perm_t;

typedef enum __packed{
	PAGE_KERNEL = 0,
	PAGE_USER
} page_priv_t;

typedef enum __packed{
	PAGE_COHERENT = 0x1,
	PAGE_NOREORDER = 0x2,
	PAGE_NOCACHE = 0x4,
	PAGE_INVAL_PROT = 0x8,
	PAGE_WRITE_THROUGH = 0x10
} page_flags_t;

typedef enum __packed{
	PAGESIZE_4k = 0x0,
	PAGESIZE_16k,
	PAGESIZE_64k,
	PAGESIZE_256k,
	PAGESIZE_1M,
	PAGESIZE_4M,
	PAGESIZE_16M,
	PAGESIZE_64M,
	PAGESIZE_256M,
	PAGESIZE_1G,
	PAGESIZE_4G
} page_size_t;


typedef struct page_t{
	/* mapping */
	page_size_t psize;							// page size
	void *phys_addr,							// physical address
		 *virt_addr;							// virtual address

	/* flags */
	unsigned int pid,							// process id
				 lpid;							// guest id

	page_flags_t flags;							// other flags
	page_priv_t priv;							// privilege level
	page_perm_t perm;							// permissions

	/* index */
	unsigned int idx;							// index in page table/TLB

	/* list handling */
	struct page_t *next,
				  *prev;
} page_t;


#endif // KERNEL_PAGE_H
