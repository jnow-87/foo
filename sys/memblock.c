/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/memblock.h>
#include <sys/list.h>
#include <sys/math.h>
#include <sys/errno.h>


/* macros */
#define MIN_BYTES_USABLE	4


/* global functions */
int memblock_init(memblock_t *pool, size_t n){
	if(pool == 0x0)
		return_errno(E_INVAL);

	list_init(pool);
	pool->len = n;

	return 0;
}

void *memblock_alloc(memblock_t **pool, size_t n, size_t align){
	memblock_t *free,
			   *blk;


	if(n == 0)
		return 0x0;

	n += sizeof(memblock_t);

	if(align)
		n = ALIGNP2(n, align);

	/* search for a free block */
	list_for_each(*pool, blk){
		// block is large enough to be split
		if(blk->len >= n + sizeof(memblock_t) + MIN_BYTES_USABLE){
			// resize blocks
			free = (void*)blk + n;
			free->len = blk->len - n;
			blk->len = n;

			// update list
			list_replace(*pool, blk, free);

			return (void*)blk + sizeof(memblock_t);
		}
		// block is large enough for n
		else if(blk->len >= n){
			// remove block from pool if it is used entirely
			list_rm(*pool, blk);

			return (void*)blk + sizeof(memblock_t);
		}
	}

	return 0x0;
}

int memblock_free(memblock_t **pool, void *addr){
	memblock_t *blk = addr - sizeof(memblock_t);
	memblock_t *el;


	if(addr == 0x0)
		return 0;

	/* search for element with higher address */
	list_for_each(*pool, el){
		// check for double free
		if(blk >= el && (void*)blk < (void*)el + el->len)
			return_errno(E_INVAL);

		if(el > blk)
			break;
	}

	/* insert blk at tail */
	if(el == 0x0){
		el = list_last(*pool);

		if(el && (void*)el + el->len == blk){
			// merge with tail
			el->len += blk->len;
		}
		else{
			// do not merge with tail
			list_add_tail(*pool, blk);
		}
	}
	/* insert blk at head */
	else if(el == list_first(*pool)){
		if((void*)blk + blk->len == el){
			// merge with head
			blk->len += el->len;
			list_replace(*pool, el, blk);
		}
		else{
			// do not merge with head
			list_add_head(*pool, blk);
		}
	}
	/* insert blk in-between two list elements */
	else{
		el = el->prev;

		if((void*)el + el->len == blk){
			// merge with front
			el->len += blk->len;
			blk = el;
		}
		else{
			// do not merge with front
			list_add_in(blk, el, el->next);
		}

		el = blk->next;

		if((void*)blk + blk->len == el){
			// merge with back
			blk->len += el->len;
			list_rm(*pool, el);
		}
	}

	return 0;
}
