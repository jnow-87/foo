/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef X86_SYSCALL_H
#define X86_SYSCALL_H


#include <sys/types.h>
#include <sys/syscall.h>


/* incomplete types */
struct thread_t;


/* types */
typedef enum{
	OLOC_NONE = 0x0,
	OLOC_PRE = 0x1,
	OLOC_POST = 0x2,
} overlay_location_t;

typedef struct{
	int (*call)(void *);
	overlay_location_t loc;
} overlay_t;


/* prototypes */
int x86_sc(sc_num_t num, void *param, size_t psize);
sc_t *x86_sc_arg(struct thread_t *this_t);
void x86_sc_epilogue(struct thread_t *this_t);
int x86_sc_overlay_call(sc_num_t num, void *param, overlay_location_t loc, overlay_t *overlays);


#endif // X86_SYSCALL_H
