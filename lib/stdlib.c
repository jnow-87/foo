#include <config/config.h>
#include <arch/syscall.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/list.h>
#include <sys/memblock.h>
#include <sys/math.h>
#include <sys/mutex.h>
#include <sys/syscall.h>


/* types */
typedef struct block_t{
	memblock_t *mem;

	struct block_t *prev,
				   *next;
} block_t;


/* static variables */
static block_t *block_lst = 0x0;
static block_t *cur_block = 0x0;

static mutex_t mem_mtx = MUTEX_INITIALISER();


/* global functions */
void *malloc(size_t size){
	sc_malloc_t p;


	mutex_lock(&mem_mtx);

	/* try to allocate in any of the available blocks */
	if(cur_block != 0x0){
		// try allocation on last used block
		p.p = memblock_alloc(&cur_block->mem, size, CONFIG_MALLOC_ALIGN);

		if(p.p != 0x0)
			goto done;

		// try allocation on any of the available blocks
		list_for_each(block_lst, cur_block){
			p.p = memblock_alloc(&cur_block->mem, size, CONFIG_MALLOC_ALIGN);

			if(p.p != 0x0)
				goto done;
		}
	}

	/* allocate from kernel */
	// adjust size
	p.size = ALIGNP2(size + sizeof(memblock_t) + sizeof(block_t), 4);
	p.size = MAX(CONFIG_MALLOC_MIN_SIZE, p.size);

	// syscall
	sc(SC_MALLOC, &p);
	errno |= p.errno;

	if(p.errno)
		goto err;

	// update block_lst
	cur_block = p.p;
	list_add_tail(block_lst, cur_block);

	cur_block->mem = p.p + sizeof(block_t);
	memblock_init(cur_block->mem, p.size);

	p.p = memblock_alloc(&cur_block->mem, size, CONFIG_MALLOC_ALIGN);

	if(p.p == 0x0)
		goto err;

done:
	mutex_unlock(&mem_mtx);
	return p.p;


err:
	mutex_unlock(&mem_mtx);
	return 0x0;
}

void free(void *p){
	block_t *blk;
	sc_malloc_t _p;


	mutex_lock(&mem_mtx);

	/* identify block associated to p */
	list_for_each(block_lst, blk){
		if((void*)blk <= p && (void*)blk->mem + blk->mem->len > p)
			break;
	}

	if(blk == 0x0)
		goto end;

	/* free p */
	if(memblock_free(&blk->mem, p) < 0)
		goto end;

	/* return block to the kernel if nothing is allocated on it */
	if((void*)blk + sizeof(block_t) != blk->mem || list_first(blk->mem) != list_last(blk->mem))
		goto end;

	list_rm(block_lst, blk);

	_p.p = blk;
	sc(SC_FREE, &_p);
	errno |= _p.errno;

end:
	mutex_unlock(&mem_mtx);
}

void exit(int status){
	sc(SC_EXIT, &status);
}
