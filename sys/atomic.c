/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <arch/arch.h>


/* types */
typedef struct{
	int volatile *v;
	int old,
		new;
} args_cas_t;

typedef struct{
	int volatile *v;
	int inc;
} args_add_t;


/* local/static prototypes */
int op_cas(void *param);
int op_add(void *param);


/* global functions */
int cas(int volatile *v, int old, int new){
	if(arch_ops_common.cas != 0x0)
		return arch_ops_common.cas(v, old, new);

	return atomic(op_cas, &((args_cas_t){ .v = v, .old = old, .new = new }));
}

void atomic_add(int volatile *v, int inc){
	if(arch_ops_common.atomic_add != 0x0)
		return arch_ops_common.atomic_add(v, inc);

	atomic(op_add, &((args_add_t){ .v = v, .inc = inc }));
}


/* local functions */
int op_cas(void *param){
	args_cas_t *p = (args_cas_t*)param;
	int t = *(p->v);


	if(t == p->old)
		*(p->v) = p->new;

	return t != p->old;
}

int op_add(void *param){
	args_add_t *p = (args_add_t*)param;


	*(p->v) += p->inc;

	return 0;
}
