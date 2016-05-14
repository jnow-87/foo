#include <sys/memblock.h>
#include <sys/list.h>
#include <sys/math.h>
#include <sys/error.h>


/* macros */
#define MIN_BYTES_USABLE	4
#define ALIGNMENT			4


/* global functions */
void memblock_init(memblock_t* pool, size_t len){
	list_init(pool);
	pool->len = len;
}

void* memblock_alloc(memblock_t** pool, size_t n){
	memblock_t *free,
			   *blk;


	if(n == 0)
		return 0;

	n = ALIGNP2(n + sizeof(memblock_t), ALIGNMENT);

	/* search for a free block */
	list_for_each(*pool, blk){
		// block is large enough to be split
		if(blk->len >= n + sizeof(memblock_t) + MIN_BYTES_USABLE){
			// resize blocks
			free = (void*)blk + n;
			free->len = blk->len - n;
			blk->len = n;

			// update list
			list_replace(pool, blk, free);

			return (void*)blk + sizeof(memblock_t);
		}
		// block is large enough for n
		else if(blk->len >= n){
			// remove block from pool if it is used entirely
			list_rm(pool, blk);

			return (void*)blk + sizeof(memblock_t);
		}
	}

	return 0x0;
}

error_t memblock_free(memblock_t** pool, void* addr){
	memblock_t *blk,
			   *el;


	if(addr == 0)
		return E_INVAL;

	/* get block address */
	blk = addr - sizeof(memblock_t);

	/* search for element with higher address */
	list_for_each(*pool, el){
		if(el > blk)
			break;
	}

	/* insert blk at head */
	if(el == list_first(*pool)){
		if((void*)blk + blk->len == el){
			// merge with head
			blk->len += el->len;
			list_replace(pool, el, blk);
		}
		else{
			// do not merge with head
			list_add_head(pool, blk);
		}
	}
	/* insert blk at tail */
	else if(el == 0x0){
		el = list_last(*pool);

		if((void*)el + el->len == blk){
			// merge with tail
			el->len += blk->len;
		}
		else{
			// do not merge with tail
			list_add_tail(pool, blk);
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
			list_add_in(pool, blk, el, el->next);
		}

		el = blk->next;

		if((void*)blk + blk->len == el){
			// merge with back
			blk->len += el->len;
			list_rm(pool, el);
		}
	}

	return E_OK;
}
