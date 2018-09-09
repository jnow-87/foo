#include <config/config.h>
#include <arch/syscall.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/list.h>
#include <sys/memblock.h>
#include <sys/math.h>
#include <sys/mutex.h>


/* types */
typedef struct block_t{
	struct block_t *prev,
				   *next;

	memblock_t *mem;
} block_t;


/* static variables */
static block_t *block_lst = 0x0;
static block_t *cur_block = 0x0;

static mutex_t mem_mtx = MUTEX_INITIALISER();


/* global functions */
void *malloc(size_t size){
	void *addr;
	sc_malloc_t p;


	mutex_lock(&mem_mtx);

	/* try to allocate in any of the available blocks */
	if(cur_block != 0x0){
		// try allocation on last used block
		addr = memblock_alloc(&cur_block->mem, size, CONFIG_MALLOC_ALIGN);

		if(addr != 0x0)
			goto clean;

		// try allocation on any of the available blocks
		list_for_each(block_lst, cur_block){
			addr = memblock_alloc(&cur_block->mem, size, CONFIG_MALLOC_ALIGN);

			if(addr != 0x0)
				goto clean;
		}
	}

	/* allocate from kernel */
	// adjust size
	p.size = ALIGNP2(size + sizeof(memblock_t) + sizeof(block_t), 4);
	p.size = MAX(CONFIG_MALLOC_MIN_SIZE, p.size);

	// syscall
	addr = 0x0;

	if(sc(SC_MALLOC, &p) != E_OK)
		goto clean;

	// update block_lst
	addr = p.p;

	cur_block = addr;
	list_add_tail(block_lst, cur_block);

	cur_block->mem = addr + sizeof(block_t);
	memblock_init(cur_block->mem, p.size);

	addr = memblock_alloc(&cur_block->mem, size, CONFIG_MALLOC_ALIGN);


clean:
	mutex_unlock(&mem_mtx);

	return addr;
}

void free(void *addr){
	block_t *blk;
	sc_malloc_t p;


	mutex_lock(&mem_mtx);

	/* identify block associated to addr */
	list_for_each(block_lst, blk){
		if((void*)blk <= addr && (void*)blk->mem + blk->mem->len > addr)
			break;
	}

	if(blk == 0x0)
		goto clean;

	/* free addr */
	if(memblock_free(&blk->mem, addr) < 0)
		goto clean;

	/* return block to the kernel if nothing is allocated on it */
	if((void*)blk + sizeof(block_t) != blk->mem || list_first(blk->mem) != list_last(blk->mem))
		goto clean;

	list_rm(block_lst, blk);

	p.p = blk;
	(void)sc(SC_FREE, &p);


clean:
	mutex_unlock(&mem_mtx);
}
