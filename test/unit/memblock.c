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
	unsigned int n;


	n = 0;

	/* invalid args */
	n += TEST_INT_EQ(memblock_init(0x0, 0), -E_INVAL);

	/* prepare element */
	memblock_init(el0, 10);

	/* check */
	n += TEST_INT_EQ(el0->len, 10);
	n += TEST_PTR_EQ(el0->prev, el0);
	n += TEST_PTR_EQ(el0->next, 0);

	return -n;
}

/**
 * \brief	invalid pool
 *
 * \return	0x0
 */
TEST(memblock_alloc_inval){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: elements */
	pool = 0x0;

	/* check */
	n += TEST_PTR_EQ(memblock_alloc(&pool, 0, 0), 0x0);
	n += TEST_PTR_EQ(memblock_alloc(&pool, 0, 4), 0x0);

	return -n;
}

/**
 * \brief	empty pool
 * 			pool with no elements
 *
 * \return	0x0
 */
TEST(memblock_alloc_empty){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: elements */
	pool = 0x0;
	INIT_EL();

	/* check */
	n += TEST_PTR_EQ(memblock_alloc(&pool, 2, 0), 0x0);
	n += TEST_PTR_EQ(memblock_alloc(&pool, 2, 4), 0x0);

	return -n;
}

/**
 * \brief	small pool
 * 			pool is not large enough for request
 *
 * \return	0x0
 */
TEST(memblock_alloc_small){
	unsigned int n;
	void *blk;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 */
	pool = 0x0;
	INIT_EL();

	list_add_head(pool, el0);

	/* alloc */
	blk = memblock_alloc(&pool, SIZE_EL0, 0);

	/* check blk */
	n += TEST_PTR_EQ(blk, 0x0);

	return -n;
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
	unsigned int n;
	void *blk;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 -> el1 -> el2 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el1);
	list_add_tail(pool, el2);

	/* alloc */
	blk = memblock_alloc(&pool, 4 + sizeof(memblock_t), 0);

	/* check blk */
	n += TEST_PTR_EQ(blk, (void*)el1 + sizeof(memblock_t));

	/*expected pool: el0 -> el2 */
	// check el0 (el1 should be removed)
	n += TEST_INT_EQ(el0->len, SIZE_EL0);
	n += TEST_PTR_EQ(el0->prev, el2);
	n += TEST_PTR_EQ(el0->next, el2);

	// check el1 (no changes)
	n += TEST_INT_EQ(el1->len, SIZE_EL1);
	n += TEST_PTR_EQ(el1->prev, el0);
	n += TEST_PTR_EQ(el1->next, el2);

	// check el2 (el1 should be removed)
	n += TEST_INT_EQ(el2->len, SIZE_EL2);
	n += TEST_PTR_EQ(el2->prev, el0);
	n += TEST_PTR_EQ(el2->next, 0x0);

	return -n;
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
	unsigned int n;
	void *blk;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 -> el3 -> el2 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el3);
	list_add_tail(pool, el2);

	/* alloc */
	blk = memblock_alloc(&pool, 4 + sizeof(memblock_t), 4);

	/* check blk */
	n += TEST_PTR_EQ(blk, (void*)el3 + sizeof(memblock_t));

	/* expected pool: el0 -> new -> el2 */
	// set blk to the new free block within pool
	blk += 4 + sizeof(memblock_t);

	// check el0 (el3 should be replaced by the new block)
	n += TEST_INT_EQ(el0->len, SIZE_EL0);
	n += TEST_PTR_EQ(el0->prev, el2);
	n += TEST_PTR_EQ(el0->next, blk);

	// check el3 (updated len)
	n += TEST_INT_EQ(el3->len, 4 + 2 * sizeof(memblock_t));
	n += TEST_PTR_EQ(el3->prev, el0);
	n += TEST_PTR_EQ(el3->next, el2);

	// check new block (remaining size of el3 and linked between el1 and el2)
	n += TEST_INT_EQ(((memblock_t*)blk)->len, SIZE_EL3 - (4 + 2 * sizeof(memblock_t)));
	n += TEST_PTR_EQ(((memblock_t*)blk)->prev, el0);
	n += TEST_PTR_EQ(((memblock_t*)blk)->next, el2);

	// check el2 (el3 should be replace by the new block)
	n += TEST_INT_EQ(el2->len, SIZE_EL2);
	n += TEST_PTR_EQ(el2->prev, blk);
	n += TEST_PTR_EQ(el2->next, 0x0);

	return -n;
}

/**
 * \brief	free head without merge
 * 			freed block is inserted at head without merging it with the
 * 			previous head
 *
 * \return	pool with 2 elements
 */
TEST(memblock_free_head_nomerge){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el2 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el2);

	/* free */
	n += TEST_INT_EQ(memblock_free(&pool, (void*)el0 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 -> el2 */
	// check el0 (linked at head of el2)
	n += TEST_INT_EQ(el0->len, SIZE_EL0);
	n += TEST_PTR_EQ(el0->prev, el2);
	n += TEST_PTR_EQ(el0->next, el2);

	// check el2 (el0 linked in front)
	n += TEST_INT_EQ(el2->len, SIZE_EL2);
	n += TEST_PTR_EQ(el2->prev, el0);
	n += TEST_PTR_EQ(el2->next, 0x0);

	return -n;
}

/**
 * \brief	free head with merge
 * 			freed block is merged with existing head
 *
 * \return	pool with 1 element and merged length
 */
TEST(memblock_free_head_merge){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el1 -> el2 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el1);
	list_add_tail(pool, el2);

	/* free */
	n += TEST_INT_EQ(memblock_free(&pool, (void*)el0 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 -> el2 */
	// check el0 (len merged with el1)
	n += TEST_INT_EQ(el0->len, SIZE_EL0 + SIZE_EL1);
	n += TEST_PTR_EQ(el0->prev, el2);
	n += TEST_PTR_EQ(el0->next, el2);

	// check el2 (el0 linked in front)
	n += TEST_INT_EQ(el2->len, SIZE_EL2);
	n += TEST_PTR_EQ(el2->prev, el0);
	n += TEST_PTR_EQ(el2->next, 0x0);

	return -n;
}

/**
 * \brief	free on empty pool
 * 			freed block is inserted at the end
 *
 * \return	pool with 1 element
 */
TEST(memblock_free_tail){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 */
	pool = 0x0;
	INIT_EL();

	/* free */
	n += TEST_INT_EQ(memblock_free(&pool, (void*)el0 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 -> el2 */
	// check el0 (el2 linked at back)
	n += TEST_INT_EQ(el0->len, SIZE_EL0);
	n += TEST_PTR_EQ(el0->prev, el0);
	n += TEST_PTR_EQ(el0->next, 0x0);

	return -n;
}

/**
 * \brief	free tail without merge
 * 			freed block is inserted at the end without merging it with
 * 			the previous tail
 *
 * \return	pool with 2 elements
 */
TEST(memblock_free_tail_nomerge){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el0);

	/* free */
	n += TEST_INT_EQ(memblock_free(&pool, (void*)el2 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 -> el2 */
	// check el0 (el2 linked at back)
	n += TEST_INT_EQ(el0->len, SIZE_EL0);
	n += TEST_PTR_EQ(el0->prev, el2);
	n += TEST_PTR_EQ(el0->next, el2);

	// check el2 (linked to el0)
	n += TEST_INT_EQ(el2->len, SIZE_EL2);
	n += TEST_PTR_EQ(el2->prev, el0);
	n += TEST_PTR_EQ(el2->next, 0x0);

	return -n;
}

/**
 * \brief	free tail with merge
 * 			freed block is merged with the previous tail
 *
 * \return	pool with 1 element and merged length
 */
TEST(memblock_free_tail_merge){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el0);

	/* free */
	n += TEST_INT_EQ(memblock_free(&pool, (void*)el1 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 */
	// check el0 (len merged with el1)
	n += TEST_INT_EQ(el0->len, SIZE_EL0 + SIZE_EL1);
	n += TEST_PTR_EQ(el0->prev, el0);
	n += TEST_PTR_EQ(el0->next, 0x0);

	return -n;
}

/**
 * \brief	free middle without merge
 * 			freed block is inserted between head and tail without merge
 *
 * \return	pool with 3 elements
 */
TEST(memblock_free_mid_nomerge){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 -> el4 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el4);

	/* free */
	n += TEST_INT_EQ(memblock_free(&pool, (void*)el2 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 -> el2 -> el4 */
	// check el0 (el2 linked at back)
	n += TEST_INT_EQ(el0->len, SIZE_EL0);
	n += TEST_PTR_EQ(el0->prev, el4);
	n += TEST_PTR_EQ(el0->next, el2);

	// check el2 (linked between el0 and el4)
	n += TEST_INT_EQ(el2->len, SIZE_EL2);
	n += TEST_PTR_EQ(el2->prev, el0);
	n += TEST_PTR_EQ(el2->next, el4);

	// check el4 (el2 linked in front)
	n += TEST_INT_EQ(el4->len, SIZE_EL4);
	n += TEST_PTR_EQ(el4->prev, el2);
	n += TEST_PTR_EQ(el4->next, 0x0);

	return -n;
}

/**
 * \brief	free middle with merge at the front
 * 			freed block is merged with the previous head
 *
 * \return	pool with 2 elements
 */
TEST(memblock_free_mid_frontmerge){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 -> el3 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el3);

	/* free */
	n += TEST_INT_EQ(memblock_free(&pool, (void*)el1 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 -> el3 */
	// check el0 (len merged with el1)
	n += TEST_INT_EQ(el0->len, SIZE_EL0 + SIZE_EL1);
	n += TEST_PTR_EQ(el0->prev, el3);
	n += TEST_PTR_EQ(el0->next, el3);

	// check el3 (no changes)
	n += TEST_INT_EQ(el3->len, SIZE_EL3);
	n += TEST_PTR_EQ(el3->prev, el0);
	n += TEST_PTR_EQ(el3->next, 0x0);

	return -n;
}

/**
 * \brief	free middle with merge at the back
 * 			freed block is merged with the previous tail
 *
 * \return	pool with 2 elements
 */
TEST(memblock_free_mid_backmerge){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 -> el3 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el3);

	/* free */
	n += TEST_INT_EQ(memblock_free(&pool, (void*)el2 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 -> el2 */
	// check el0 (linked el2 at back)
	n += TEST_INT_EQ(el0->len, SIZE_EL0);
	n += TEST_PTR_EQ(el0->prev, el2);
	n += TEST_PTR_EQ(el0->next, el2);

	// check el2 (linked as tail and merged with el3)
	n += TEST_INT_EQ(el2->len, SIZE_EL2 + SIZE_EL3);
	n += TEST_PTR_EQ(el2->prev, el0);
	n += TEST_PTR_EQ(el2->next, 0x0);

	return -n;
}

/**
 * \brief	free middle with merge at both sides
 * 			freed block is merged with the previous head
 *
 * \return	pool with 1 element and merged length
 */
TEST(memblock_free_mid_merge){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 -> el1 -> el2 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el2);

	/* free */
	n += TEST_INT_EQ(memblock_free(&pool, (void*)el1 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 */
	// check el0 (linked el2 at back)
	n += TEST_INT_EQ(el0->len, SIZE_EL0 + SIZE_EL1 + SIZE_EL2);
	n += TEST_PTR_EQ(el0->prev, el0);
	n += TEST_PTR_EQ(el0->next, 0x0);

	return -n;
}

/**
 * \brief	large pool first split by separate allocations
 * 			and merged afterwards through free
 */
TEST(memblock_cycle){
	unsigned int n;
	void *blk0,
		 *blk1,
		 *blk2;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 -> el1 -> el2 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el3);

	/* alloc */
	blk0 = memblock_alloc(&pool, 4, 0);
	blk1 = memblock_alloc(&pool, 4, 4);
	blk2 = memblock_alloc(&pool, 4, 8);

	/* addresses expected to be in line */
	n += TEST_PTR_EQ(blk0, (void*)el3 + sizeof(memblock_t));
	n += TEST_PTR_EQ(blk1, (void*)el3 + 2 * sizeof(memblock_t) + 4);
	n += TEST_PTR_EQ(blk2, (void*)el3 + 3 * sizeof(memblock_t) + 8);

	/* expected pool: empty */
	n += TEST_PTR_EQ(pool, 0x0);

	/* free blk2 */
	n += TEST_INT_EQ(memblock_free(&pool, blk2), E_OK);

	/* expected pool: 1 element */
	n += TEST_PTR_EQ(pool, blk2 - sizeof(memblock_t));
	n += TEST_INT_EQ(pool->len, SIZE_EL3 - (2 * sizeof(memblock_t) + 8));
	n += TEST_PTR_EQ(pool->prev, pool);
	n += TEST_PTR_EQ(pool->next, 0x0);

	/* free blk0 */
	n += TEST_INT_EQ(memblock_free(&pool, blk0), E_OK);

	/* expected pool: 2 elements */
	n += TEST_PTR_EQ(pool, blk0 - sizeof(memblock_t));

	// check 1st pool entry
	n += TEST_INT_EQ(pool->len, 4 + sizeof(memblock_t));
	n += TEST_PTR_EQ(pool->prev, blk2 - sizeof(memblock_t));
	n += TEST_PTR_EQ(pool->next, blk2 - sizeof(memblock_t));

	// check 2nd pool entry
	n += TEST_INT_EQ(pool->next->len, SIZE_EL3 - (2 * sizeof(memblock_t) + 8));
	n += TEST_PTR_EQ(pool->next->prev, pool);
	n += TEST_PTR_EQ(pool->next->next, 0x0);

	/* free blk1 */
	n += TEST_INT_EQ(memblock_free(&pool, blk1), E_OK);

	/* expected pool: 1 element */
	n += TEST_PTR_EQ(pool, blk0 - sizeof(memblock_t));
	n += TEST_INT_EQ(pool->len, SIZE_EL3);
	n += TEST_PTR_EQ(pool->prev, pool);
	n += TEST_PTR_EQ(pool->next, 0x0);

	return -n;
}

/**
 * \brief	zero free
 * 			call free with 0x0
 *
 * \return	E_OK
 */
TEST(memblock_zero_free){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: empty */
	pool = 0x0;
	INIT_EL();

	/* 2nd free of el0 */
	n += TEST_INT_EQ(memblock_free(&pool, 0x0), E_OK);

	return -n;
}

/**
 * \brief	double free
 * 			a previously freed block is freed again
 *
 * \return	pool with 2 elements
 */
TEST(memblock_double_free){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: empty */
	pool = 0x0;
	INIT_EL();

	/* 1st free of el0 */
	n += TEST_INT_EQ(memblock_free(&pool, (void*)el0 + sizeof(memblock_t)), E_OK);

	/* 2nd free of el0 */
	n += TEST_INT_EQ(memblock_free(&pool, (void*)el0 + sizeof(memblock_t)), -E_INVAL);

	return -n;
}
