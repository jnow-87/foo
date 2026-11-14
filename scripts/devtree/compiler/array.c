/**
 * Copyright (C) 2026 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <stdlib.h>
#include <sys/map.h>
#include <sys/string.h>
#include <sys/types.h>
#include <sys/vector.h>
#include <parser.tab.h>
#include "array.h"
#include "expr.h"



/* local/static prototypes */
static size_t hash(void *key);
static int kmatch(void *key0, void *key1);
static int kvalloc(void **key, void **value);
static void kvfree(void *key, void *value);


/* static variables */
static map_t array_cache = MAP_INITIALISER(hash, kmatch, kvalloc, kvfree);


/* global functions */
array_t *array_cache_add(array_t **arr){
	array_t *r;


	r = map_add(&array_cache, *arr, *arr);

	if(r == 0x0)
		devtree_parser_error("out of memory");

	// ensure the original array is "consumed", i.e. either it has been moved to the
	// cache or the cache already contains a matching entry, in which case the original
	// array is no longer needed and thus has to be freed
	if(r != *arr)
		array_free(*arr);

	return (*arr = r);
}

void array_cache_free(void){
	map_reset(&array_cache);
}

array_t *array_alloc(void){
	array_t *arr;


	arr = malloc(sizeof(array_t));

	if(arr != 0x0){
		arr->items = VECTOR_INITIALISER(sizeof(expr_value_t));
		arr->cached = false;
	}

	return arr;
}

void array_free(array_t *arr){
	vector_reset(&arr->items);
	free(arr);
}

array_t *array_add(array_t **arr, void *val){
	array_t *dup;


	if((*arr)->cached){
		dup = malloc(sizeof(array_t));

		if(dup == 0x0)
			goto err_0;

		if(vector_dup(&dup->items, &(*arr)->items) != 0)
			goto err_1;

		dup->cached = false;

		*arr = dup;
	}

	if(vector_add(&(*arr)->items, val) != 0)
		goto err_1;

	return *arr;


err_1:
	array_free(*arr);

err_0:
	return (*arr = 0x0);
}


/* local functions */
static size_t hash(void *key){
	size_t h = 0;
	array_t *arr = (array_t*)key;
	expr_value_t *v;


	vector_for_each(&arr->items, v)
		h += v->is_array + v->i;

	return h;
}

static int kmatch(void *key0, void *key1){
	array_t *k0 = (array_t*)key0,
			*k1 = (array_t*)key1;
	vector_t *v0 = &k0->items,
			 *v1 = &k1->items;


	if(v0->dt_size != v1->dt_size || v0->size != v1->size)
		return -1;

	return memcmp(v0->buf, v1->buf, v0->dt_size * v0->size);
}

static int kvalloc(void **key, void **val){
	(*((array_t**)key))->cached = true;
	*val = *key;

	return 0;
}

static void kvfree(void *key, void *val){
	array_free(key);
}
