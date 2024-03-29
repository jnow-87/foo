/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_STACK_H
#define SYS_STACK_H


#include <sys/compiler.h>
#include <sys/list.h>


/* static variables */
static list1_t *dummy_tail __unused = 0x0;	// a stack does not require a tail pointer
											// hence a dummy is used when calling the
											// list implementation


/* macros */
/**
 * \brief	initialise the stack
 *
 * \param	top		pointer to the stack
 */
#define stack_init(top) \
	list1_init(top, dummy_tail)

/**
 * \brief	get the top element on the stack without
 * 			removing it
 *
 * \param	top		pointer to the stack
 *
 * \return	pointer to the top element
 */
#define stack_top(top)({ \
	LIST_TYPE_COMPAT(*top); \
	\
	top; \
})

/**
 * \brief	add an element to the stack
 *
 * \param	top		pointer to the stack
 * \param	el		pointer to the target element
 */
#define stack_push(top, el) \
	list1_add_head(top, dummy_tail, el); \

/**
 * \brief	remove an element from the stack, returning
 * 			it's address
 *
 * \param	top		pointer to the stack
 *
 * \return	pointer to the element
 */
#define stack_pop(top)({ \
	typeof(top) _el; \
	\
	\
	_el = list_first(top); \
	\
	if(_el != 0x0) \
		list1_rm_head(top, dummy_tail); \
	\
	_el; \
})


#endif // SYS_STACK_H
