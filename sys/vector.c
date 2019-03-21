/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/vector.h>
#include <sys/string.h>

#ifdef BUILD_KERNEL

#include <kernel/memory.h>

#define	_alloc	kmalloc
#define _free	kfree

#else

#ifdef BUILD_HOST
#include <stdlib.h>
#else
#include <lib/stdlib.h>
#endif // BUILD_HOST

#define _alloc malloc
#define _free free

#endif // BUILD_KERNEL


/* global functions */
int vector_init(vector_t *v, size_t dt_size, size_t capa){
	v->data = _alloc(dt_size * capa);

	if(v->data == 0x0)
		return -1;

	v->capacity = capa;
	v->size = 0;
	v->dt_size = dt_size;

	return 0;
}

void vector_destroy(vector_t *v){
	_free(v->data);

	v->capacity = 0;
	v->size = 0;
}

int vector_add(vector_t *v, void *data){
	void *t;


	if(v->size >= v->capacity){
		v->capacity *= 2;
		t = _alloc(v->capacity * v->dt_size);

		if(t == 0x0)
			return -1;

		memcpy(t, v->data, v->size * v->dt_size);
		_free(v->data);
		v->data = t;
	}

	memcpy(v->data + v->size * v->dt_size, data, v->dt_size);
	v->size++;

	return 0;
}

void vector_rm(vector_t *v, size_t idx){
	if(idx >= v->size)
		return;

	memcpy(v->data + idx * v->dt_size, v->data + (idx + 1) * v->dt_size, (v->size - idx - 1) * v->dt_size);
	v->size--;
}

void *vector_get(vector_t *v, size_t idx){
	if(idx >= v->size)
		return 0x0;

	return v->data + idx * v->dt_size;
}
