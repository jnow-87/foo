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
#include <testing/testcase.h>


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
static int tc_memblock_print(int log){
	INIT_EL();

	tlog(log, "el0 addr: %#x, size: %u\n", el0, el0->len);
	tlog(log, "el1 addr: %#x, size: %u\n", el1, el1->len);
	tlog(log, "el2 addr: %#x, size: %u\n", el2, el2->len);
	tlog(log, "el3 addr: %#x, size: %u\n", el3, el3->len);
	tlog(log, "el4 addr: %#x, size: %u\n", el4, el4->len);

	return 0;
}

test_case(tc_memblock_print, "memblock: addresses");


/**
 * \brief	init pool
 *
 * \return 0x0
 */
static int tc_memblock_init(int log){
	unsigned int n;


	n = 0;

	/* invalid args */
	n += check_int(log, memblock_init(0x0, 0), -E_INVAL);

	/* prepare element */
	memblock_init(el0, 10);

	/* check */
	n += check_int(log, el0->len, 10);
	n += check_ptr(log, el0->prev, el0);
	n += check_ptr(log, el0->next, 0);

	return -n;
}

test_case(tc_memblock_init, "memblock_init");


/**
 * \brief	invalid pool
 *
 * \return	0x0
 */
static int tc_memblock_alloc_inval(int log){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: elements */
	pool = 0x0;

	/* check */
	n += check_ptr(log, memblock_alloc(&pool, 0, 0), 0x0);
	n += check_ptr(log, memblock_alloc(&pool, 0, 4), 0x0);

	return -n;
}

test_case(tc_memblock_alloc_inval, "memblock_alloc: invalid args");


/**
 * \brief	empty pool
 * 			pool with no elements
 *
 * \return	0x0
 */
static int tc_memblock_alloc_empty(int log){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: elements */
	pool = 0x0;
	INIT_EL();

	/* check */
	n += check_ptr(log, memblock_alloc(&pool, 2, 0), 0x0);
	n += check_ptr(log, memblock_alloc(&pool, 2, 4), 0x0);

	return -n;
}

test_case(tc_memblock_alloc_empty, "memblock_alloc: empty pool");


/**
 * \brief	small pool
 * 			pool is not large enough for request
 *
 * \return	0x0
 */
static int tc_memblock_alloc_small(int log){
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
	n += check_ptr(log, blk, 0x0);

	return -n;
}

test_case(tc_memblock_alloc_small, "memblock_alloc: small pool");


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
static int tc_memblock_alloc_perfect_fit(int log){
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
	n += check_ptr(log, blk, (void*)el1 + sizeof(memblock_t));

	/*expected pool: el0 -> el2 */
	// check el0 (el1 should be removed)
	n += check_int(log, el0->len, SIZE_EL0);
	n += check_ptr(log, el0->prev, el2);
	n += check_ptr(log, el0->next, el2);

	// check el1 (no changes)
	n += check_int(log, el1->len, SIZE_EL1);
	n += check_ptr(log, el1->prev, el0);
	n += check_ptr(log, el1->next, el2);

	// check el2 (el1 should be removed)
	n += check_int(log, el2->len, SIZE_EL2);
	n += check_ptr(log, el2->prev, el0);
	n += check_ptr(log, el2->next, 0x0);

	return -n;
}

test_case(tc_memblock_alloc_perfect_fit, "memblock_alloc: perfect fit");


/**
 * \brief	split fit
 * 			pool contains 3 elements, with the 2nd being large enough to
 * 			satisfy the request and create a new block for additional
 * 			allocations
 *
 * \return	address within 2nd element
 * 			2nd element is split
 */
static int tc_memblock_alloc_split_fit(int log){
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
	n += check_ptr(log, blk, (void*)el3 + sizeof(memblock_t));

	/* expected pool: el0 -> new -> el2 */
	// set blk to the new free block within pool
	blk += 4 + sizeof(memblock_t);

	// check el0 (el3 should be replaced by the new block)
	n += check_int(log, el0->len, SIZE_EL0);
	n += check_ptr(log, el0->prev, el2);
	n += check_ptr(log, el0->next, blk);

	// check el3 (updated len)
	n += check_int(log, el3->len, 4 + 2 * sizeof(memblock_t));
	n += check_ptr(log, el3->prev, el0);
	n += check_ptr(log, el3->next, el2);

	// check new block (remaining size of el3 and linked between el1 and el2)
	n += check_int(log, ((memblock_t*)blk)->len, SIZE_EL3 - (4 + 2 * sizeof(memblock_t)));
	n += check_ptr(log, ((memblock_t*)blk)->prev, el0);
	n += check_ptr(log, ((memblock_t*)blk)->next, el2);

	// check el2 (el3 should be replace by the new block)
	n += check_int(log, el2->len, SIZE_EL2);
	n += check_ptr(log, el2->prev, blk);
	n += check_ptr(log, el2->next, 0x0);

	return -n;
}

test_case(tc_memblock_alloc_split_fit, "memblock_alloc: split fit");


/**
 * \brief	free head without merge
 * 			freed block is inserted at head without merging it with the
 * 			previous head
 *
 * \return	pool with 2 elements
 */
static int tc_memblock_free_head_nomerge(int log){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el2 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el2);

	/* free */
	n += check_int(log, memblock_free(&pool, (void*)el0 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 -> el2 */
	// check el0 (linked at head of el2)
	n += check_int(log, el0->len, SIZE_EL0);
	n += check_ptr(log, el0->prev, el2);
	n += check_ptr(log, el0->next, el2);

	// check el2 (el0 linked in front)
	n += check_int(log, el2->len, SIZE_EL2);
	n += check_ptr(log, el2->prev, el0);
	n += check_ptr(log, el2->next, 0x0);

	return -n;
}

test_case(tc_memblock_free_head_nomerge, "memblock_free: head no merge");


/**
 * \brief	free head with merge
 * 			freed block is merged with existing head
 *
 * \return	pool with 1 element and merged length
 */
static int tc_memblock_free_head_merge(int log){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el1 -> el2 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el1);
	list_add_tail(pool, el2);

	/* free */
	n += check_int(log, memblock_free(&pool, (void*)el0 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 -> el2 */
	// check el0 (len merged with el1)
	n += check_int(log, el0->len, SIZE_EL0 + SIZE_EL1);
	n += check_ptr(log, el0->prev, el2);
	n += check_ptr(log, el0->next, el2);

	// check el2 (el0 linked in front)
	n += check_int(log, el2->len, SIZE_EL2);
	n += check_ptr(log, el2->prev, el0);
	n += check_ptr(log, el2->next, 0x0);

	return -n;
}

test_case(tc_memblock_free_head_merge, "memblock_free: head merge");

/**
 * \brief	free on empty pool
 * 			freed block is inserted at the end
 *
 * \return	pool with 1 element
 */
static int tc_memblock_free_tail(int log){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 */
	pool = 0x0;
	INIT_EL();

	/* free */
	n += check_int(log, memblock_free(&pool, (void*)el0 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 -> el2 */
	// check el0 (el2 linked at back)
	n += check_int(log, el0->len, SIZE_EL0);
	n += check_ptr(log, el0->prev, el0);
	n += check_ptr(log, el0->next, 0x0);

	return -n;
}

test_case(tc_memblock_free_tail, "memblock_free: tail");



/**
 * \brief	free tail without merge
 * 			freed block is inserted at the end without merging it with
 * 			the previous tail
 *
 * \return	pool with 2 elements
 */
static int tc_memblock_free_tail_nomerge(int log){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el0);

	/* free */
	n += check_int(log, memblock_free(&pool, (void*)el2 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 -> el2 */
	// check el0 (el2 linked at back)
	n += check_int(log, el0->len, SIZE_EL0);
	n += check_ptr(log, el0->prev, el2);
	n += check_ptr(log, el0->next, el2);

	// check el2 (linked to el0)
	n += check_int(log, el2->len, SIZE_EL2);
	n += check_ptr(log, el2->prev, el0);
	n += check_ptr(log, el2->next, 0x0);

	return -n;
}

test_case(tc_memblock_free_tail_nomerge, "memblock_free: tail no merge");


/**
 * \brief	free tail with merge
 * 			freed block is merged with the previous tail
 *
 * \return	pool with 1 element and merged length
 */
static int tc_memblock_free_tail_merge(int log){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el0);

	/* free */
	n += check_int(log, memblock_free(&pool, (void*)el1 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 */
	// check el0 (len merged with el1)
	n += check_int(log, el0->len, SIZE_EL0 + SIZE_EL1);
	n += check_ptr(log, el0->prev, el0);
	n += check_ptr(log, el0->next, 0x0);

	return -n;
}

test_case(tc_memblock_free_tail_merge, "memblock_free: tail merge");


/**
 * \brief	free middle without merge
 * 			freed block is inserted between head and tail without merge
 *
 * \return	pool with 3 elements
 */
static int tc_memblock_free_mid_nomerge(int log){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 -> el4 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el4);

	/* free */
	n += check_int(log, memblock_free(&pool, (void*)el2 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 -> el2 -> el4 */
	// check el0 (el2 linked at back)
	n += check_int(log, el0->len, SIZE_EL0);
	n += check_ptr(log, el0->prev, el4);
	n += check_ptr(log, el0->next, el2);

	// check el2 (linked between el0 and el4)
	n += check_int(log, el2->len, SIZE_EL2);
	n += check_ptr(log, el2->prev, el0);
	n += check_ptr(log, el2->next, el4);

	// check el4 (el2 linked in front)
	n += check_int(log, el4->len, SIZE_EL4);
	n += check_ptr(log, el4->prev, el2);
	n += check_ptr(log, el4->next, 0x0);

	return -n;
}

test_case(tc_memblock_free_mid_nomerge, "memblock_free: mid no merge");


/**
 * \brief	free middle with merge at the front
 * 			freed block is merged with the previous head
 *
 * \return	pool with 2 elements
 */
static int tc_memblock_free_mid_frontmerge(int log){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 -> el3 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el3);

	/* free */
	n += check_int(log, memblock_free(&pool, (void*)el1 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 -> el3 */
	// check el0 (len merged with el1)
	n += check_int(log, el0->len, SIZE_EL0 + SIZE_EL1);
	n += check_ptr(log, el0->prev, el3);
	n += check_ptr(log, el0->next, el3);

	// check el3 (no changes)
	n += check_int(log, el3->len, SIZE_EL3);
	n += check_ptr(log, el3->prev, el0);
	n += check_ptr(log, el3->next, 0x0);

	return -n;
}

test_case(tc_memblock_free_mid_frontmerge, "memblock_free: mid merge front");


/**
 * \brief	free middle with merge at the back
 * 			freed block is merged with the previous tail
 *
 * \return	pool with 2 elements
 */
static int tc_memblock_free_mid_backmerge(int log){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 -> el3 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el3);

	/* free */
	n += check_int(log, memblock_free(&pool, (void*)el2 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 -> el2 */
	// check el0 (linked el2 at back)
	n += check_int(log, el0->len, SIZE_EL0);
	n += check_ptr(log, el0->prev, el2);
	n += check_ptr(log, el0->next, el2);

	// check el2 (linked as tail and merged with el3)
	n += check_int(log, el2->len, SIZE_EL2 + SIZE_EL3);
	n += check_ptr(log, el2->prev, el0);
	n += check_ptr(log, el2->next, 0x0);

	return -n;
}

test_case(tc_memblock_free_mid_backmerge, "memblock_free: mid merge back");

/**
 * \brief	free middle with merge at both sides
 * 			freed block is merged with the previous head
 *
 * \return	pool with 1 element and merged length
 */
static int tc_memblock_free_mid_merge(int log){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: el0 -> el1 -> el2 */
	pool = 0x0;
	INIT_EL();

	list_add_tail(pool, el0);
	list_add_tail(pool, el2);

	/* free */
	n += check_int(log, memblock_free(&pool, (void*)el1 + sizeof(memblock_t)), E_OK);

	/* expected pool: el0 */
	// check el0 (linked el2 at back)
	n += check_int(log, el0->len, SIZE_EL0 + SIZE_EL1 + SIZE_EL2);
	n += check_ptr(log, el0->prev, el0);
	n += check_ptr(log, el0->next, 0x0);

	return -n;
}

test_case(tc_memblock_free_mid_merge, "memblock_free: mid merge front and back");


/**
 * \brief	large pool first split by separate allocations
 * 			and merged afterwards through free
 */
int tc_memblock_cycle(int log){
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
	n += check_ptr(log, blk0, (void*)el3 + sizeof(memblock_t));
	n += check_ptr(log, blk1, (void*)el3 + 2 * sizeof(memblock_t) + 4);
	n += check_ptr(log, blk2, (void*)el3 + 3 * sizeof(memblock_t) + 8);

	/* expected pool: empty */
	n += check_ptr(log, pool, 0x0);

	/* free blk2 */
	n += check_int(log, memblock_free(&pool, blk2), E_OK);

	/* expected pool: 1 element */
	n += check_ptr(log, pool, blk2 - sizeof(memblock_t));
	n += check_int(log, pool->len, SIZE_EL3 - (2 * sizeof(memblock_t) + 8));
	n += check_ptr(log, pool->prev, pool);
	n += check_ptr(log, pool->next, 0x0);

	/* free blk0 */
	n += check_int(log, memblock_free(&pool, blk0), E_OK);

	/* expected pool: 2 elements */
	n += check_ptr(log, pool, blk0 - sizeof(memblock_t));

	// check 1st pool entry
	n += check_int(log, pool->len, 4 + sizeof(memblock_t));
	n += check_ptr(log, pool->prev, blk2 - sizeof(memblock_t));
	n += check_ptr(log, pool->next, blk2 - sizeof(memblock_t));

	// check 2nd pool entry
	n += check_int(log, pool->next->len, SIZE_EL3 - (2 * sizeof(memblock_t) + 8));
	n += check_ptr(log, pool->next->prev, pool);
	n += check_ptr(log, pool->next->next, 0x0);

	/* free blk1 */
	n += check_int(log, memblock_free(&pool, blk1), E_OK);

	/* expected pool: 1 element */
	n += check_ptr(log, pool, blk0 - sizeof(memblock_t));
	n += check_int(log, pool->len, SIZE_EL3);
	n += check_ptr(log, pool->prev, pool);
	n += check_ptr(log, pool->next, 0x0);

	return -n;
}

test_case(tc_memblock_cycle, "memblock: entire cycle alloc, free");


/**
 * \brief	zero free
 * 			call free with 0x0
 *
 * \return	E_OK
 */
static int tc_memblock_zero_free(int log){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: empty */
	pool = 0x0;
	INIT_EL();

	/* 2nd free of el0 */
	n += check_int(log, memblock_free(&pool, 0x0), E_OK);

	return -n;
}

test_case(tc_memblock_zero_free, "memblock_free: zero free");


/**
 * \brief	double free
 * 			a previously freed block is freed again
 *
 * \return	pool with 2 elements
 */
static int tc_memblock_double_free(int log){
	unsigned int n;
	memblock_t *pool;


	n = 0;

	/* prepare pool: empty */
	pool = 0x0;
	INIT_EL();

	/* 1st free of el0 */
	n += check_int(log, memblock_free(&pool, (void*)el0 + sizeof(memblock_t)), E_OK);

	/* 2nd free of el0 */
	n += check_int(log, memblock_free(&pool, (void*)el0 + sizeof(memblock_t)), -E_INVAL);

	return -n;
}

test_case(tc_memblock_double_free, "memblock_free: double free");
