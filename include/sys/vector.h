/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef VECTOR_H
#define VECTOR_H


#include <sys/types.h>


/* macros */
#define vector_for_each(v, p) \
	for(p=(v)->data; p<(typeof(p))((v)->data+(v)->dt_size*(v)->size); p++)


/* types */
typedef struct{
	void *data;

	size_t dt_size;

	size_t capacity,
		   size;
} vector_t;


/* prototypes */
int vector_init(vector_t *v, size_t dt_size, size_t capa);
void vector_destroy(vector_t *v);

int vector_add(vector_t *v, void *data);
void vector_rm(vector_t *v, size_t idx);

void *vector_get(vector_t *v, size_t idx);


#endif // VECTOR_H
