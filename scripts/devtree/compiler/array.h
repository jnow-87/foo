/**
 * Copyright (C) 2026 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef ARRAY_H
#define ARRAY_H


#include <sys/types.h>
#include <sys/vector.h>


/* macros */
#define ARRAY_INITIALISER(dt_size) ((array_t){ \
	.items = VECTOR_INITIALISER(dt_size), \
	.cached = false, \
})


/* types */
typedef struct{
	vector_t items;
	bool cached;
} array_t;


/* prototypes */
array_t *array_cache_add(array_t **arr);
void array_cache_free(void);

array_t *array_alloc(size_t type_size);
void array_free(array_t *arr);
array_t *array_add(array_t **arr, void *val);


#endif // ARRAY_H
