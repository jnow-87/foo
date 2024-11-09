/**
 * Copyright (C) 2024 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef DEVTREE_ATTR_H
#define DEVTREE_ATTR_H


#include <sys/types.h>


/* macros */
#define ATTR_VALUE(type, value) \
	(attr_value_t){ .type = value }


/* types */
typedef enum{
	MT_COMPATIBLE = 1,
	MT_ADDR,
	MT_STRING,
	MT_INT8,
	MT_INT16,
	MT_INT32,
	MT_INT64,
	MT_SIZE,
	MT_ADDR_WIDTH,
	MT_REG_WIDTH,
	MT_NCORES,
	MT_NUM_INTS,
	MT_TIMER_INT,
	MT_SYSCALL_INT,
	MT_IPI_INT,
	MT_TIMER_CYCLE_TIME_US,
	MT_CORE_MASK,
} attr_type_t;

typedef union{
	void *p;
	unsigned long int i;
} attr_value_t;

typedef struct{
	attr_type_t type;
	attr_value_t value;
} attr_t;


/* prototypes */
char const *attr_name(attr_type_t type);


#endif // DEVTREE_ATTR_H
