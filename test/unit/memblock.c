/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <config/config.h>
#include <sys/memblock.h>
#include <sys/list.h>
#include <sys/errno.h>
#include <test/test.h>


/* macros */
#define SIZE_EL0	(2 * sizeof(memblock_t) + 2)
#define SIZE_EL1	(2 * sizeof(memblock_t) + 4)
#define SIZE_EL2	(2 * sizeof(memblock_t) + 8)
#define SIZE_EL3	(3 * sizeof(memblock_t) + 16)
#define SIZE_EL4	(3 * sizeof(memblock_t) + 16)

#define INIT_EL() \
	el0->prev = el0->next = (void*)0x1; \
	el0->len = SIZE_EL0; \
	el1->prev = el1->next = (void*)0x1; \
	el1->len = SIZE_EL1; \
	el2->prev = el2->next = (void*)0x1; \
	el2->len = SIZE_EL2; \
	el3->prev = el3->next = (void*)0x1; \
	el3->len = SIZE_EL3; \
	el4->prev = el4->next = (void*)0x1; \
	el4->len = SIZE_EL4; \


/* static variables */
static char el_blk[SIZE_EL0 + SIZE_EL1 + SIZE_EL2 + SIZE_EL3 + SIZE_EL4];

static memblock_t *el0 = (memblock_t*)((void*)el_blk + 0x0),
				  *el1 = (memblock_t*)((void*)el_blk + SIZE_EL0),
				  *el2 = (memblock_t*)((void*)el_blk + SIZE_EL0 + SIZE_EL1),
				  *el3 = (memblock_t*)((void*)el_blk + SIZE_EL0 + SIZE_EL1 + SIZE_EL2),
				  *el4 = (memblock_t*)((void*)el_blk + SIZE_EL0 + SIZE_EL1 + SIZE_EL2 + SIZE_EL3);


/* local functions */
TEST(memblock_addresses){
	INIT_EL();

	TEST_LOG("el0 addr: %#x, size: %u\n", el0, el0->len);
	TEST_LOG("el1 addr: %#x, size: %u\n", el1, el1->len);
	TEST_LOG("el2 addr: %#x, size: %u\n", el2, el2->len);
	TEST_LOG("el3 addr: %#x, size: %u\n", el3, el3->len);
	TEST_LOG("el4 addr: %#x, size: %u\n", el4, el4->len);

	return 0;
}

/**
 * \brief	init pool
 *
 * \return 0x0
 */
TEST(memblock_init){
	int r = 0;


	/* invalid args */
	r += TEST_INT_EQ(memblock_init(0x0, 0), -E_INVAL);

	/* prepare element */
	memblock_init(el0, 10);

	/* check */
	r += TEST_INT_EQ(el0->len, 10);
	r += TEST_PTR_EQ(el0->prev, el0);
	r += TEST_PTR_EQ(el0->next, 0);

	return -r;
}

/**
 * \brief	invalid pool
 *
 * \return	0x0
 */
TEST(memblock_alloc_inval){
	int r = 0;
	memblock_t *pool = 0x0;


	r += TEST_PTR_EQ(memblock_alloc(&pool, 0, 0), 0x0);
	r += TEST_PTR_EQ(memblock_alloc(&pool, 0, 4), 0x0);

	return -r;
}

/**
 * \brief	empty pool
 * 			pool with no elements
 *
 * \return	0x0
 */
TEST(memblock_alloc_empty){
	int r = 0;
	memblock_t *pool = 0x0;


	INIT_EL();

	r += TEST_PTR_EQ(memblock_alloc(&pool, 2, 0), 0x0);
	r += TEST_PTR_EQ(memblock_alloc(&pool, 2, 4), 0x0);

	return -r;
}

/**
 * \brief	small pool
 * 			pool is not large enough for request
 *
 * \return	0x0
 */
TEST(memblock_alloc_small){
	int r = 0;
	memblock_t *pool = 0x0;
	void *blk;


	/* prepare pool: el0 */
	INIT_EL();

	list_add_head(pool, el0);

	/* alloc */
	blk = memblock_alloc(&pool, SIZE_EL0, 0);

	/* check blk */
	r += TEST_PTR_EQ(blk, 0x0);

	return -r;
}

/**
 * \brief	perfect fit
 * 			pool contains 3 elements, with the 2nd being large enough to
 * 			satisfy the request, without having enough space to allow an
 * 			additional allocation
 *
 * \return  address within the 2nd element
 * 			2nd element is removed from the pool list, linking 1st
 * 			and 3rd
 */
TEST(memblock_alloc_perfect_fit){
	int r = 0;
	memblock_t *pool = 0x0;
	void *blk;


	/* prepare pool: el0 -> el1 -> el2 */
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el1);
	list_add_tail(pool, el2);

	/* alloc */
	blk = memblock_alloc(&pool, 4 + sizeof(memblock_t), 0);

	/* check blk */
	r += TEST_PTR_EQ(blk, (void*)el1 + sizeof(memblock_t));

	/*expected pool: el0 -> el2 */
	// check el0 (el1 should be removed)
	r += TEST_INT_EQ(el0->len, SIZE_EL0);
	r += TEST_PTR_EQ(el0->prev, el2);
	r += TEST_PTR_EQ(el0->next, el2);

	// check el1 (no changes)
	r += TEST_INT_EQ(el1->len, SIZE_EL1);
	r += TEST_PTR_EQ(el1->prev, el0);
	r += TEST_PTR_EQ(el1->next, el2);

	// check el2 (el1 should be removed)
	r += TEST_INT_EQ(el2->len, SIZE_EL2);
	r += TEST_PTR_EQ(el2->prev, el0);
	r += TEST_PTR_EQ(el2->next, 0x0);

	return -r;
}

/**
 * \brief	split fit
 * 			pool contains 3 elements, with the 2nd being large enough to
 * 			satisfy the request and create a new block for additional
 * 			allocations
 *
 * \return	address within 2nd element
 * 			2nd element is split
 */
TEST(memblock_alloc_split_fit){
	int r = 0;
	memblock_t *pool = 0x0;
	void *blk;


	/* prepare pool: el0 -> el3 -> el2 */
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el3);
	list_add_tail(pool, el2);

	/* alloc */
	blk = memblock_alloc(&pool, 4 + sizeof(memblock_t), 4);

	/* check blk */
	r += TEST_PTR_EQ(blk, (void*)el3 + sizeof(memblock_t));

	/* expected pool: el0 -> new -> el2 */
	// set blk to the new free block within pool
	blk += 4 + sizeof(memblock_t);

	// check el0 (el3 should be replaced by the new block)
	r += TEST_INT_EQ(el0->len, SIZE_EL0);
	r += TEST_PTR_EQ(el0->prev, el2);
	r += TEST_PTR_EQ(el0->next, blk);

	// check el3 (updated len)
	r += TEST_INT_EQ(el3->len, 4 + 2 * sizeof(memblock_t));
	r += TEST_PTR_EQ(el3->prev, el0);
	r += TEST_PTR_EQ(el3->next, el2);

	// check new block (remaining size of el3 and linked between el1 and el2)
	r += TEST_INT_EQ(((memblock_t*)blk)->len, SIZE_EL3 - (4 + 2 * sizeof(memblock_t)));
	r += TEST_PTR_EQ(((memblock_t*)blk)->prev, el0);
	r += TEST_PTR_EQ(((memblock_t*)blk)->next, el2);

	// check el2 (el3 should be replace by the new block)
	r += TEST_INT_EQ(el2->len, SIZE_EL2);
	r += TEST_PTR_EQ(el2->prev, blk);
	r += TEST_PTR_EQ(el2->next, 0x0);

	return -r;
}

/**
 * \brief	free head without merge
 * 			freed block is inserted at head without merging it with the
 * 			previous head
 *
 * \return	pool with 2 elements
 */
TEST(memblock_free_head_nomerge){
	int r = 0;
	memblock_t *pool = 0x0;


	/* prepare pool: el2 */
	INIT_EL();

	list_add_tail(pool, el2);

	/* free */
	r += TEST_INT_EQ(memblock_free(&pool, (void*)el0 + sizeof(memblock_t)), 0);

	/* expected pool: el0 -> el2 */
	// check el0 (linked at head of el2)
	r += TEST_INT_EQ(el0->len, SIZE_EL0);
	r += TEST_PTR_EQ(el0->prev, el2);
	r += TEST_PTR_EQ(el0->next, el2);

	// check el2 (el0 linked in front)
	r += TEST_INT_EQ(el2->len, SIZE_EL2);
	r += TEST_PTR_EQ(el2->prev, el0);
	r += TEST_PTR_EQ(el2->next, 0x0);

	return -r;
}

/**
 * \brief	free head with merge
 * 			freed block is merged with existing head
 *
 * \return	pool with 1 element and merged length
 */
TEST(memblock_free_head_merge){
	int r = 0;
	memblock_t *pool = 0x0;


	/* prepare pool: el1 -> el2 */
	INIT_EL();

	list_add_tail(pool, el1);
	list_add_tail(pool, el2);

	/* free */
	r += TEST_INT_EQ(memblock_free(&pool, (void*)el0 + sizeof(memblock_t)), 0);

	/* expected pool: el0 -> el2 */
	// check el0 (len merged with el1)
	r += TEST_INT_EQ(el0->len, SIZE_EL0 + SIZE_EL1);
	r += TEST_PTR_EQ(el0->prev, el2);
	r += TEST_PTR_EQ(el0->next, el2);

	// check el2 (el0 linked in front)
	r += TEST_INT_EQ(el2->len, SIZE_EL2);
	r += TEST_PTR_EQ(el2->prev, el0);
	r += TEST_PTR_EQ(el2->next, 0x0);

	return -r;
}

/**
 * \brief	free on empty pool
 * 			freed block is inserted at the end
 *
 * \return	pool with 1 element
 */
TEST(memblock_free_tail){
	int r = 0;
	memblock_t *pool = 0x0;


	/* prepare pool: el0 */
	INIT_EL();

	/* free */
	r += TEST_INT_EQ(memblock_free(&pool, (void*)el0 + sizeof(memblock_t)), 0);

	/* expected pool: el0 -> el2 */
	// check el0 (el2 linked at back)
	r += TEST_INT_EQ(el0->len, SIZE_EL0);
	r += TEST_PTR_EQ(el0->prev, el0);
	r += TEST_PTR_EQ(el0->next, 0x0);

	return -r;
}

/**
 * \brief	free tail without merge
 * 			freed block is inserted at the end without merging it with
 * 			the previous tail
 *
 * \return	pool with 2 elements
 */
TEST(memblock_free_tail_nomerge){
	int r = 0;
	memblock_t *pool = 0x0;


	/* prepare pool: el0 */
	INIT_EL();

	list_add_tail(pool, el0);

	/* free */
	r += TEST_INT_EQ(memblock_free(&pool, (void*)el2 + sizeof(memblock_t)), 0);

	/* expected pool: el0 -> el2 */
	// check el0 (el2 linked at back)
	r += TEST_INT_EQ(el0->len, SIZE_EL0);
	r += TEST_PTR_EQ(el0->prev, el2);
	r += TEST_PTR_EQ(el0->next, el2);

	// check el2 (linked to el0)
	r += TEST_INT_EQ(el2->len, SIZE_EL2);
	r += TEST_PTR_EQ(el2->prev, el0);
	r += TEST_PTR_EQ(el2->next, 0x0);

	return -r;
}

/**
 * \brief	free tail with merge
 * 			freed block is merged with the previous tail
 *
 * \return	pool with 1 element and merged length
 */
TEST(memblock_free_tail_merge){
	int r = 0;
	memblock_t *pool = 0x0;


	/* prepare pool: el0 */
	INIT_EL();

	list_add_tail(pool, el0);

	/* free */
	r += TEST_INT_EQ(memblock_free(&pool, (void*)el1 + sizeof(memblock_t)), 0);

	/* expected pool: el0 */
	// check el0 (len merged with el1)
	r += TEST_INT_EQ(el0->len, SIZE_EL0 + SIZE_EL1);
	r += TEST_PTR_EQ(el0->prev, el0);
	r += TEST_PTR_EQ(el0->next, 0x0);

	return -r;
}

/**
 * \brief	free middle without merge
 * 			freed block is inserted between head and tail without merge
 *
 * \return	pool with 3 elements
 */
TEST(memblock_free_mid_nomerge){
	int r = 0;
	memblock_t *pool = 0x0;


	/* prepare pool: el0 -> el4 */
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el4);

	/* free */
	r += TEST_INT_EQ(memblock_free(&pool, (void*)el2 + sizeof(memblock_t)), 0);

	/* expected pool: el0 -> el2 -> el4 */
	// check el0 (el2 linked at back)
	r += TEST_INT_EQ(el0->len, SIZE_EL0);
	r += TEST_PTR_EQ(el0->prev, el4);
	r += TEST_PTR_EQ(el0->next, el2);

	// check el2 (linked between el0 and el4)
	r += TEST_INT_EQ(el2->len, SIZE_EL2);
	r += TEST_PTR_EQ(el2->prev, el0);
	r += TEST_PTR_EQ(el2->next, el4);

	// check el4 (el2 linked in front)
	r += TEST_INT_EQ(el4->len, SIZE_EL4);
	r += TEST_PTR_EQ(el4->prev, el2);
	r += TEST_PTR_EQ(el4->next, 0x0);

	return -r;
}

/**
 * \brief	free middle with merge at the front
 * 			freed block is merged with the previous head
 *
 * \return	pool with 2 elements
 */
TEST(memblock_free_mid_frontmerge){
	int r = 0;
	memblock_t *pool = 0x0;


	/* prepare pool: el0 -> el3 */
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el3);

	/* free */
	r += TEST_INT_EQ(memblock_free(&pool, (void*)el1 + sizeof(memblock_t)), 0);

	/* expected pool: el0 -> el3 */
	// check el0 (len merged with el1)
	r += TEST_INT_EQ(el0->len, SIZE_EL0 + SIZE_EL1);
	r += TEST_PTR_EQ(el0->prev, el3);
	r += TEST_PTR_EQ(el0->next, el3);

	// check el3 (no changes)
	r += TEST_INT_EQ(el3->len, SIZE_EL3);
	r += TEST_PTR_EQ(el3->prev, el0);
	r += TEST_PTR_EQ(el3->next, 0x0);

	return -r;
}

/**
 * \brief	free middle with merge at the back
 * 			freed block is merged with the previous tail
 *
 * \return	pool with 2 elements
 */
TEST(memblock_free_mid_backmerge){
	int r = 0;
	memblock_t *pool = 0x0;


	/* prepare pool: el0 -> el3 */
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el3);

	/* free */
	r += TEST_INT_EQ(memblock_free(&pool, (void*)el2 + sizeof(memblock_t)), 0);

	/* expected pool: el0 -> el2 */
	// check el0 (linked el2 at back)
	r += TEST_INT_EQ(el0->len, SIZE_EL0);
	r += TEST_PTR_EQ(el0->prev, el2);
	r += TEST_PTR_EQ(el0->next, el2);

	// check el2 (linked as tail and merged with el3)
	r += TEST_INT_EQ(el2->len, SIZE_EL2 + SIZE_EL3);
	r += TEST_PTR_EQ(el2->prev, el0);
	r += TEST_PTR_EQ(el2->next, 0x0);

	return -r;
}

/**
 * \brief	free middle with merge at both sides
 * 			freed block is merged with the previous head
 *
 * \return	pool with 1 element and merged length
 */
TEST(memblock_free_mid_merge){
	int r = 0;
	memblock_t *pool = 0x0;


	/* prepare pool: el0 -> el1 -> el2 */
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el2);

	/* free */
	r += TEST_INT_EQ(memblock_free(&pool, (void*)el1 + sizeof(memblock_t)), 0);

	/* expected pool: el0 */
	// check el0 (linked el2 at back)
	r += TEST_INT_EQ(el0->len, SIZE_EL0 + SIZE_EL1 + SIZE_EL2);
	r += TEST_PTR_EQ(el0->prev, el0);
	r += TEST_PTR_EQ(el0->next, 0x0);

	return -r;
}

/**
 * \brief	large pool first split by separate allocations
 * 			and merged afterwards through free
 */
TEST(memblock_cycle){
	int r = 0;
	memblock_t *pool = 0x0;
	void *blk0,
		 *blk1,
		 *blk2;


	/* prepare pool: el0 -> el1 -> el2 */
	INIT_EL();

	list_add_tail(pool, el3);

	/* alloc */
	blk0 = memblock_alloc(&pool, 4, 0);
	blk1 = memblock_alloc(&pool, 4, 4);
	blk2 = memblock_alloc(&pool, 4, 8);

	/* addresses expected to be in line */
	r += TEST_PTR_EQ(blk0, (void*)el3 + sizeof(memblock_t));
	r += TEST_PTR_EQ(blk1, (void*)el3 + 2 * sizeof(memblock_t) + 4);
	r += TEST_PTR_EQ(blk2, (void*)el3 + 3 * sizeof(memblock_t) + 8);

	/* expected pool: empty */
	r += TEST_PTR_EQ(pool, 0x0);

	/* free blk2 */
	r += TEST_INT_EQ(memblock_free(&pool, blk2), 0);

	/* expected pool: 1 element */
	r += TEST_PTR_EQ(pool, blk2 - sizeof(memblock_t));
	r += TEST_INT_EQ(pool->len, SIZE_EL3 - (2 * sizeof(memblock_t) + 8));
	r += TEST_PTR_EQ(pool->prev, pool);
	r += TEST_PTR_EQ(pool->next, 0x0);

	/* free blk0 */
	r += TEST_INT_EQ(memblock_free(&pool, blk0), 0);

	/* expected pool: 2 elements */
	r += TEST_PTR_EQ(pool, blk0 - sizeof(memblock_t));

	// check 1st pool entry
	r += TEST_INT_EQ(pool->len, 4 + sizeof(memblock_t));
	r += TEST_PTR_EQ(pool->prev, blk2 - sizeof(memblock_t));
	r += TEST_PTR_EQ(pool->next, blk2 - sizeof(memblock_t));

	// check 2nd pool entry
	r += TEST_INT_EQ(pool->next->len, SIZE_EL3 - (2 * sizeof(memblock_t) + 8));
	r += TEST_PTR_EQ(pool->next->prev, pool);
	r += TEST_PTR_EQ(pool->next->next, 0x0);

	/* free blk1 */
	r += TEST_INT_EQ(memblock_free(&pool, blk1), 0);

	/* expected pool: 1 element */
	r += TEST_PTR_EQ(pool, blk0 - sizeof(memblock_t));
	r += TEST_INT_EQ(pool->len, SIZE_EL3);
	r += TEST_PTR_EQ(pool->prev, pool);
	r += TEST_PTR_EQ(pool->next, 0x0);

	return -r;
}

/**
 * \brief	zero free
 * 			call free with 0x0
 *
 * \return	0
 */
TEST(memblock_zero_free){
	int r = 0;
	memblock_t *pool = 0x0;


	/* prepare pool: empty */
	INIT_EL();

	/* 2nd free of el0 */
	r += TEST_INT_EQ(memblock_free(&pool, 0x0), 0);

	return -r;
}

/**
 * \brief	double free
 * 			a previously freed block is freed again
 *
 * \return	pool with 2 elements
 */
TEST(memblock_double_free){
	int r = 0;
	memblock_t *pool = 0x0;


	/* prepare pool: empty */
	INIT_EL();

	/* 1st free of el0 */
	r += TEST_INT_EQ(memblock_free(&pool, (void*)el0 + sizeof(memblock_t)), 0);

	/* 2nd free of el0 */
	r += TEST_INT_EQ(memblock_free(&pool, (void*)el0 + sizeof(memblock_t)), -E_INVAL);

	return -r;
}
