/**
 * Copyright (C) 2026 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/list.h>
#include <sys/map.h>
#include <sys/math.h>
#include <sys/memory.h>
#include <sys/types.h>


/* macros */
#define OVERLOADED(size, capa)	((size) >= ((8 * (capa)) / 9))
#define HASH(map, key)			((map)->hash(key) % (map)->capacity)


/* local/static prototypes */
static void add(map_t *map, map_entry_t *e);
static int rehash(map_t *map);


/* global functions */
void map_free(map_t *map){
	map_entry_t *e;


	for(size_t i=0; i<map->capacity; i++){
		list_for_each(map->table[i], e){
			map->kvfree(e->key, e->value);
			sys_free(e);
		}
	}

	sys_free(map->table);
	*map = MAP_INITIALISER(map->hash, map->kmatch, map->kvalloc, map->kvfree);
}

int map_add(map_t *map, void *key, void *value){
	map_entry_t *e;


	if(OVERLOADED(map->size + 1, map->capacity)){
		if(rehash(map) != 0)
			goto err_0;
	}

	if(map_lookup(map, key) != 0x0)
		return 1;

	e = sys_malloc(sizeof(map_entry_t));

	if(e == 0x0)
		goto err_0;

	e->key = key;
	e->value = value;

	if(map->kvalloc(&e->key, &e->value) != 0)
		goto err_1;

	add(map, e);

	return 0;


err_1:
	sys_free(e);

err_0:
	return -1;
}

void *map_lookup(map_t *map, void *key){
	size_t i = HASH(map, key);
	map_entry_t *e;


	list_for_each(map->table[i], e){
		if(map->kmatch(key, e->key) == 0)
			return e->value;
	}

	return 0x0;
}


/* local functions */
static void add(map_t *map, map_entry_t *e){
	size_t i = HASH(map, e->key);


	list_add_tail(map->table[i], e);
	map->size++;
}

static int rehash(map_t *map){
	map_t old = *map;
	map_entry_t *e;


	while(OVERLOADED(map->size + 1, map->capacity))
		map->capacity = prime_next(MAX(map->capacity, (size_t)7));

	map->table = sys_calloc(map->capacity, sizeof(map_entry_t*));

	if(map->table == 0x0){
		map->capacity = old.capacity;
		map->table = old.table;

		return -1;
	}

	map->size = 0;

	for(size_t i=0; i<old.capacity; i++){
		list_for_each(old.table[i], e)
			add(map, e);
	}

	sys_free(old.table);

	return 0;
}
