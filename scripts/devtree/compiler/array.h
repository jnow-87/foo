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


/* types */
typedef struct{
	vector_t items;
	bool cached;
} array_t;


/* prototypes */
array_t *array_cache_add(array_t **arr);
void array_cache_free(void);

array_t *array_alloc(void);
void array_free(array_t *arr);
array_t *array_add(array_t **arr, void *val);


#endif // ARRAY_H
