/**
 * Copyright (C) 2019 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_DEVTREE_H
#define SYS_DEVTREE_H

#ifndef ASM

#ifndef BUILD_HOST
# include <sys/types.h>
#else
# include <stdint.h>
# include <stdio.h>
#endif // BUILD_HOST
#endif // ASM

#include <sys/devicetree.h>


/* macros */
#if DEVTREE_ARCH_NCORES > 1
# define DEVTREE_ARCH_MULTI_CORE	1
#endif

#define DEVTREE_ARCH_CORE_MASK		((0x1 << DEVTREE_ARCH_NCORES) - 1)


/* types */
#ifndef ASM
typedef struct devtree_node_t{
	char const *name,
			   *compatible;

	void const *payload;

	struct devtree_node_t const * const *childs;
} devtree_node_t;

// TODO may remove mem and arch payloads, if they can be defined via the types in the device tree
// TODO check where devtree_arch_payload() was used 'vim /devtree_find(.*,platform' and try to replace macro uses with a static expression
typedef struct{
	void *base;
	uint32_t size;
} devtree_memory_payload_t;

typedef struct{
	uint8_t addr_width,
			reg_width;
	uint8_t ncores;

	uint8_t num_ints;
	uint8_t timer_int,
			syscall_int,
			ipi_int;

	uint32_t timer_cycle_time_us;
} devtree_arch_payload_t;

typedef int (*devtree_action_t)(devtree_node_t const *node, size_t level, void *data);
typedef int (*devtree_filter_t)(devtree_node_t const *node);
#endif // ASM


/* prototypes */
#ifndef ASM
void const *devtree_find(char const *comp, char const *name);
void devtree_traverse(devtree_action_t action, devtree_filter_t filter, void *data);

int devtree_filter_device(devtree_node_t const *node);
int devtree_filter_memory(devtree_node_t const *node);
#endif // ASM


/* external variables */
#ifndef ASM
extern devtree_node_t const __dt_root;
#endif // ASM


#endif // SYS_DEVTREE_H
