#ifndef SYS_MEMBLOCK_H
#define SYS_MEMBLOCK_H


#include <sys/types.h>
#include <sys/errno.h>


/* types */
typedef struct memblock_t{
	/**
	 * \brief	type to represent a block of memory
	 * 				- its starting address equals the address of the
	 * 				  memblock_t instance
	 * 				- its len member represents the entire size of the block,
	 * 				  i.e. usable size + sizeof(memblock_t)
	 */

	size_t len;

	struct memblock_t *prev,
					  *next;
} memblock_t;


/* prototypes */
errno_t memblock_init(memblock_t *pool, size_t len);
void *memblock_alloc(memblock_t **pool, size_t n);
void memblock_free(memblock_t **pool, void *addr);


#endif // SYS_MEMBLOCK_H
