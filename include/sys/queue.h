/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#ifndef SYS_QUEUE_H
#define SYS_QUEUE_H


#include <sys/list.h>

/* macros */
/**
 * \brief	initialise head and tail elements intented
 * 			to represent a queue
 *
 * \param	head	pointer to head element (valid
 * 					pointer target required)
 *
 * \param	tail	tail pointer (must not point reference
 * 					an instance of the target type)
 */
#define queue_init(head, tail) \
	list1_init(head, tail)

/**
 * \brief	add an element to the queue
 *
 * \param	head	pointer to the queue head
 * \param	tail	pointer to the queue tail
 * \param	el		pointer to the target element
 */
#define queue_enqueue(head, tail, el) \
	list1_add_tail(head, tail, el)

/**
 * \brief	remove an element from the queue, returning
 * 			it's address
 *
 * \param	head	pointer to the queue head
 * \param	tail	poitner to the queue tail
 *
 * \return	pointer to the element
 */
#define queue_dequeue(head, tail)({ \
	typeof(head) _el; \
	\
	\
	_el = list_first(head); \
	\
	if(_el != 0x0) \
		list1_rm_head(head, tail); \
	\
	_el; \
})


#endif // SYS_QUEUE_H
