/**
 * Copyright (C) 2026 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef MAP_H
#define MAP_H


#include <sys/types.h>


/* macros */
#define MAP_INITIALISER(hash_cb, key_match_cb, kv_alloc_cb, kv_free_cb) (map_t){ \
	.capacity = 0, \
	.size = 0, \
	.table = 0x0, \
	.hash = hash_cb, \
	.kmatch = key_match_cb, \
	.kvalloc = kv_alloc_cb, \
	.kvfree = kv_free_cb, \
}


/* types */
typedef struct map_entry_t{
	struct map_entry_t *prev,
					   *next;

	void *key,
		 *value;
} map_entry_t;

typedef struct{
	size_t capacity,
		   size;

	map_entry_t **table;

	size_t (*hash)(void *key);
	int (*kmatch)(void *key0, void *key1);
	int (*kvalloc)(void **key, void **value);
	void (*kvfree)(void *key, void *value);
} map_t;


/* prototypes */
void map_free(map_t *map);

int map_add(map_t *map, void *key, void *value);
void *map_lookup(map_t *map, void *key);


#endif // MAP_H
