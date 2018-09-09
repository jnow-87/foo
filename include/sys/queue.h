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
#define queue_init(head, tail){ \
	LIST_TYPE_COMPAT(*head); \
	LIST_TYPE_COMPAT(*tail); \
	_list1_init((list1_t*)head, (list1_t**)&tail); \
}

/**
 * \brief	add an element to the queue
 *
 * \param	head	pointer to the queue head
 * \param	tail	pointer to the queue tail
 * \param	el		pointer to the target element
 */
#define queue_enqueue(head, tail, el){ \
	LIST_TYPE_COMPAT(*head); \
	LIST_TYPE_COMPAT(*tail); \
	LIST_TYPE_COMPAT(*el); \
	_list1_add_tail((list1_t**)&head, (list1_t**)&tail, (list1_t*)el); \
}

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
	LIST_TYPE_COMPAT(*head); \
	LIST_TYPE_COMPAT(*tail); \
	typeof(head) _el; \
	\
	_el = list_first(head); \
	\
	if(_el != 0x0) \
		_list1_rm_head((list1_t**)&head, (list1_t**)&tail); \
	\
	_el; \
})


#endif // SYS_QUEUE_H
