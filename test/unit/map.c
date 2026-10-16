/**
 * Copyright (C) 2026 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <sys/limits.h>
#include <sys/map.h>
#include <sys/string.h>
#include <test/memory.h>
#include <test/test.h>


/* local/static prototypes */
static size_t hash(void *key);

static int kmatch(void *key0, void *key1);
static int kvalloc(void **key, void **val);
static void kvfree(void *key, void *val);

static int alloc_mock(size_t n, size_t size);


/* static variables */
static enum{
	FAIL_DISABLED = 0,
	FAIL_TABLE,
	FAIL_TABLE_ENTRY,
	FAIL_KEY_VALUE,
} alloc_fail = FAIL_DISABLED;

static struct{
	size_t kvalloc_called,
		   kvfree_called;

	int alloc_mock_fail;
} stats;


/* local functions */
TEST(map_cycle){
	int r = 0;
	map_t map = MAP_INITIALISER(hash, kmatch, kvalloc, kvfree);
	char *s;


	memset(&stats, 0, sizeof(stats));

	// add multiple keys whereas one is attempted twice, which must fail
	r |= TEST_INT_EQ(map_add(&map, "ab", ""), 0);
	r |= TEST_INT_EQ(map_add(&map, "ba", ""), 0);
	r |= TEST_INT_EQ(map_add(&map, "ba", ""), 1);
	r |= TEST_INT_EQ(map.size, 2);

	// lookup a non-existing key
	r |= TEST_PTR_EQ(map_lookup(&map, "foo"), 0x0);

	// lookup the successfully added keys
	ASSERT_PTR_NEQ((s = map_lookup(&map, "ab")), 0x0); r |= TEST_STR_EQ(s, "ab");
	ASSERT_PTR_NEQ((s = map_lookup(&map, "ba")), 0x0); r |= TEST_STR_EQ(s, "ba");

	map_free(&map);

	r |= TEST_INT_EQ(stats.kvalloc_called, 2);
	r |= TEST_INT_EQ(stats.kvfree_called, 2);

	return -r;
}

TEST(map_rehash){
	int r = 0;
	map_t map = MAP_INITIALISER(hash, kmatch, kvalloc, kvfree);
	char text[] = "a";
	char *s;
	size_t capa;


	memset(&stats, 0, sizeof(stats));

	// ensure at least two keys map to the same map entry
	r |= TEST_INT_EQ(map_add(&map, "ab", ""), 0);
	r |= TEST_INT_EQ(map_add(&map, "ba", ""), 0);
	r |= TEST_INT_NEQ(map.capacity, 0);
	capa = map.capacity;

	// add new key until the map capacity changes, i.e. a rehash happened
	while(capa == map.capacity){
		r |= TEST_INT_EQ(map_add(&map, text, ""), 0);
		text[0] += 1;
	}

	// ensure all previous keys are also in the rehashed table
	ASSERT_PTR_NEQ((s = map_lookup(&map, "ab")), 0x0); r |= TEST_STR_EQ(s, "ab");
	ASSERT_PTR_NEQ((s = map_lookup(&map, "ba")), 0x0); r |= TEST_STR_EQ(s, "ba");

	text[0] = 'a';

	for(size_t i=0; i<map.size-2; i++){
		ASSERT_PTR_NEQ((s = map_lookup(&map, text)), 0x0);
		r |= TEST_STR_EQ(s, text);
		text[0] += 1;
	}

	// ensure rehash does not reallocate any keys
	r |= TEST_INT_EQ(stats.kvalloc_called, map.size);

	map_free(&map);

	return -r;
}

TEST(map_out_of_memory){
	int r = 0;
	map_t map = MAP_INITIALISER(hash, kmatch, kvalloc, kvfree);


	memset(&stats, 0, sizeof(stats));
	memmock_alloc_fail_callback = alloc_mock;

	// fail table allocation
	alloc_fail = FAIL_TABLE;
	r |= TEST_INT_EQ(map_add(&map, "f", ""), -1);
	r |= TEST_INT_EQ(stats.alloc_mock_fail, FAIL_TABLE);
	r |= TEST_INT_EQ(map.capacity, 0);
	r |= TEST_INT_EQ(map.size, 0);

	// fail table entry allocation
	alloc_fail = FAIL_TABLE_ENTRY;
	r |= TEST_INT_EQ(map_add(&map, "f", ""), -1);
	r |= TEST_INT_EQ(stats.alloc_mock_fail, FAIL_TABLE_ENTRY);
	r |= TEST_INT_NEQ(map.capacity, 0);
	r |= TEST_INT_EQ(map.size, 0);

	// fail key-value allocation
	alloc_fail = FAIL_KEY_VALUE;
	r |= TEST_INT_EQ(map_add(&map, "f", ""), -1);
	r |= TEST_INT_EQ(stats.alloc_mock_fail, FAIL_KEY_VALUE);
	r |= TEST_INT_NEQ(map.capacity, 0);
	r |= TEST_INT_EQ(map.size, 0);

	return -r;
}

static size_t hash(void *key){
	size_t v = 0;


	for(char *s=key; *s!=0; s++)
		v += *s;

	return v;
}

static int kmatch(void *key0, void *key1){
	return strcmp(key0, key1);
}

static int kvalloc(void **key, void **val){
	stats.kvalloc_called++;

	*key = strdup(*key);
	*val = *key;

	return (*key != 0x0) ? 0 : -1;
}

static void kvfree(void *key, void *val){
	stats.kvfree_called++;

	free(key);
}

static int alloc_mock(size_t n, size_t size){
	size *= n;
	stats.alloc_mock_fail = alloc_fail;

	switch(alloc_fail){
	case FAIL_TABLE:		return (size < sizeof(map_entry_t) + 1);
	case FAIL_TABLE_ENTRY:	return (size != sizeof(map_entry_t));
	case FAIL_KEY_VALUE:	return (size >= sizeof(map_entry_t));	// assuming small test strings
	case FAIL_DISABLED:		return -1;
	default:				return 0;
	}
}
