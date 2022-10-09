/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_MEMBLOCK_H
#define SYS_MEMBLOCK_H


#include <sys/types.h>
#include <sys/errno.h>


/* types */
/**
 * \brief	type to represent a block of memory
 * 				- its starting address equals the address of the
 * 				  memblock_t instance
 * 				- its len member represents the entire size of the block,
 * 				  i.e. usable size + sizeof(memblock_t)
 */
typedef struct memblock_t{
	struct memblock_t *prev,
					  *next;

	size_t len;
} memblock_t;


/* prototypes */
int memblock_init(memblock_t *pool, size_t n);
void *memblock_alloc(memblock_t **pool, size_t n, size_t align);
int memblock_free(memblock_t **pool, void *addr);


#endif // SYS_MEMBLOCK_H
