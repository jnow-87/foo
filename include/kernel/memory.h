/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef KERNEL_MEM_H
#define KERNEL_MEM_H


#include <config/config.h>
#include <sys/devicetree.h>

#ifndef ASM
# include <sys/types.h>
#endif // ASM


/* macros */
#define KERNEL_STACK_ASM(core)	(DEVTREE_HEAP_BASE + DEVTREE_HEAP_SIZE - (core * CONFIG_STACK_SIZE))

#ifndef ASM
// cast KERNEL_STACK_ASM() twice since for some architectures, e.g. avr, the ram addresses
// have an artificial offset to the actual hardware addresses, which, if casted directly
// to a pointer, causes a warning due to different sizes of integer and pointer
# define KERNEL_STACK(core)		((void*)(ptrdiff_t)KERNEL_STACK_ASM(core))
#endif // ASM

#define PAGESIZE_BYTES(psize)	((0x1 << (2 * (psize))) * 4096)

# ifndef CONFIG_KERNEL_MEMCHECK
#  define memcheck_stack_prime(stack)	{}
#  define memcheck_stack_check(stack)	{}
# endif // CONFIG_KERNEL_MEMCHECK


/* incomplete types */
#ifndef ASM
struct process_t;
#endif // ASM


/* types */
#ifndef ASM
# ifdef CONFIG_KERNEL_VIRT_MEM
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
	PAGESIZE_MIN = 0x0,
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
	PAGESIZE_MAX = PAGESIZE_1G
} page_size_t;
# endif // CONFIG_KERNEL_VIRT_MEM

# ifdef CONFIG_KERNEL_NO_VIRT_MEM
typedef size_t page_size_t;
# endif // CONFIG_KERNEL_NO_VIRT_MEM

typedef struct page_t{
	/* list handling */
	struct page_t *prev,
				  *next;

	/* mapping */
	void *phys_addr;							// physical address

# ifdef CONFIG_KERNEL_VIRT_MEM
	void *virt_addr;							// virtual address

	page_size_t psize;							// page size
# endif // CONFIG_KERNEL_VIRT_MEM

	/* flags */
# ifdef CONFIG_KERNEL_VIRT_MEM
	page_flags_t flags;							// other flags
	page_priv_t priv;							// privilege level
	page_perm_t perm;							// permissions
# endif // CONFIG_KERNEL_VIRT_MEM

	/* index */
# ifdef CONFIG_KERNEL_VIRT_MEM
	unsigned int idx;							// index in page table/TLB
# endif // CONFIG_KERNEL_VIRT_MEM
} page_t;
#endif //ASM


/* prototypes */
#ifndef ASM
void *kmalloc(size_t n);
void *kpalloc(size_t n);
void *kcalloc(size_t n, size_t size);
void kfree(void *addr);

bool iskheap(void *addr);

void *kmmap(void *addr);
void kmunmap(void *addr);

page_t *page_alloc(struct process_t *this_p, page_size_t psize);
void page_free(struct process_t *this_p, page_t *page);

int copy_from_user(void *kernel, void const *user, size_t n, struct process_t *this_p);
int copy_to_user(void *user, void const *kernel, size_t n, struct process_t *this_p);

# ifdef CONFIG_KERNEL_VIRT_MEM
void *addr_virt_to_phys(void *va);
void *addr_phys_to_virt(void *pa);
# endif // CONFIG_KERNEL_VIRT_MEM

# ifdef CONFIG_KERNEL_MEMCHECK
void memcheck_stack_prime(page_t *stack);
void memcheck_stack_check(page_t *stack);
# endif // CONFIG_KERNEL_MEMCHECK
#endif // ASM


#endif // KERNEL_MEM_H
