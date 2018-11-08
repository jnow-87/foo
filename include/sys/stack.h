/*
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_STACK_H
#define SYS_STACK_H


#include <sys/list.h>


/* static variables */
static list1_t *dummy_tail = 0x0;	// a stack does not require a tail pointer
									// hence a dummy is used when calling the
									// list implementation


/* macros */
/**
 * \brief	initialise the stack
 *
 * \param	top		pointer to the first stack element
 * 					(valid pointer target required)
 */
#define stack_init(top){ \
	LIST_TYPE_COMPAT(*top); \
	_list1_init((list1_t*)top, &dummy_tail); \
}

/**
 * \brief	add an element to the stack
 *
 * \param	top		pointer to the stack
 * \param	el		pointer to the target element
 */
#define stack_push(top, el){ \
	LIST_TYPE_COMPAT(*top); \
	LIST_TYPE_COMPAT(*el); \
	_list1_add_head((list1_t**)&top, &dummy_tail, (list1_t*)el); \
}

/**
 * \brief	remove an element from the stack, returning
 * 			it's address
 *
 * \param	top		pointer to the stack
 *
 * \return	pointer to the element
 */
#define stack_pop(top)({ \
	LIST_TYPE_COMPAT(*top); \
	typeof(top) _el; \
	\
	\
	_el = list_first(top); \
	\
	if(_el != 0x0) \
		_list1_rm_head((list1_t**)&top, &dummy_tail); \
	\
	_el; \
})


#endif // SYS_STACK_H
