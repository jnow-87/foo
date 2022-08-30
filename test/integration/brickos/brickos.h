/**
 * Copyright (C) 2020 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef TEST_INT_BRICKOS_H
#define TEST_INT_BRICKOS_H


#include <sys/types.h>
#include <brickos/child.h>


/* macros */
#define KERNEL	(brickos_childs[BOS_KERNEL])
#define APP		(brickos_childs[BOS_APP])


/* types */
typedef enum{
	BOS_KERNEL = 0,
	BOS_APP,
	BOS_NCHILDS
} brickos_child_t;


/* prototypes */
int brickos_init_childs(void);
void brickos_destroy_childs(void);

char const *brickos_child_name(pid_t pid);


/* external variables */
extern child_t *brickos_childs[BOS_NCHILDS];


#endif // TEST_INT_BRICKOS_H
