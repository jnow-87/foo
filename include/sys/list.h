#ifndef SYS_LIST_H
#define SYS_LIST_H


#include <sys/string.h>
#include <sys/mutex.h>

/**
 * \brief	macros to handle a double-linked list
 * 			they can be used with any structure that
 * 			contains a next and prev pointer with
 * 			type of the struct
 *
 * this version of the list macros uses the head element as functional
 * part of the list, i.e. it also stores data. therefor, the list head
 * is required to be a pointer
 * head->prev points to the last element of the list, while the list
 * is delimited by 0 with head->prev->next = 0
 */


/**
 * \brief	execute expr in a critical section protected by mutex mtx
 *
 * \param	mtx		mutex to use for protecting the section
 * \param	expr	expression to protect
 */
#define LOCK_SECTION(mtx, expr){ \
	mutex_lock(mtx); \
	expr; \
	mutex_unlock(mtx); \
}

/**
 * \brief	initialize the head of a list
 *
 * \param	el	pointer to list element
 *
 * \return	none
 */
#define list_init_el(el){ \
	if((el) != 0){ \
		(el)->prev = (el); \
		(el)->next = 0; \
	} \
}

/**
 * \brief	add an element at head of the list
 *
 * \param	head	pointer to list head
 * \param	el		pointer to element to insert
 *
 * \return	none
 */
#define list_add_head(head, el){ \
	typeof(el) _el = (el); \
	\
	\
	_el->next = (head); \
	_el->prev = ((head) == 0 ? _el : (head)->prev); \
	if((head) != 0) \
		(head)->prev = _el; \
	\
	(head) = _el; \
}

/**
 * \brief	thread-safe variant of list_add_head()
 *
 * \param	head	cf. list_add_head()
 * \param	el		cf. list_add_head()
 * \param	mtx		mutex to lock
 *
 * \return	none
 */
#define list_add_head_safe(head, el, mtx) \
	LOCK_SECTION(mtx, list_add_head(head, el))

/**
 * \brief	add an element at the end of the list
 *
 * \param	head	pointer to list head
 * \param	el		pointer to element to insert
 *
 * \return	none
 */
#define list_add_tail(head, el){ \
	typeof(el) _el = (el); \
	\
	\
	_el->next = 0; \
	if((head) == 0){ \
		(head) = _el; \
		_el->prev = _el; \
	} \
	else{ \
		_el->prev = (head)->prev; \
		(head)->prev->next = _el; \
		(head)->prev = _el; \
	} \
}

/**
 * \brief	thread-safe variant of list_add_tail()
 *
 * \param	head	cf. list_add_tail()
 * \param	el		cf. list_add_tail()
 * \param	mtx		mutex to lock
 *
 * \return	none
 */
#define list_add_tail_safe(head, el, mtx) \
	LOCK_SECTION(mtx, list_add_tail((head), (el)))

/**
 * \brief	add an element between the two given elements
 *
 * \param	el		pointer to element to insert
 * \param	front	pointer to element that is in front of el
 * \param	back	pointer to element that is after el
 *
 * \return	none
 */
#define list_add_in(el, front, back){ \
	typeof(el) _el = el; \
	typeof(el) _front = front; \
	typeof(el) _back = back; \
	\
	\
	_el->prev = front; \
	_el->next = back; \
	_front->next = _el; \
	_back->prev = _el; \
}

/**
 * \brief	thread-safe variant of list_add_in()
 *
 * \param	el		cf. list_add_in()
 * \param	front	cf. list_add_in()
 * \param	back	cf. list_add_in()
 * \param	mtx		mutex to lock
 *
 * \return	none
 */
#define list_add_in_safe(el, front, back, mtx) \
	LOCK_SECTION(mtx, list_add_in(el, front, back))

/**
 * \brief	remove an element and insert a new one at its location
 *
 * \param	head	pointer to list head
 * \param	old		element to replace
 * \param	new		element to insert
 *
 * \return	none
 */
#define list_replace(head, old, new){ \
	typeof(new) _new = (new); \
	\
	\
	if((head) == 0){ \
		(head) = _new; \
		_new->prev = _new; \
		_new->next = 0; \
	} \
	else{ \
		_new->prev = ((head)->next == 0) ? _new : (old)->prev; \
		_new->next = (old)->next; \
		\
		if((old)->next)		(old)->next->prev = _new; \
		else				(head)->prev = _new; \
		\
		if((old) != (head))	(old)->prev->next = _new; \
		else				(head) = _new; \
	} \
}

/**
 * \brief	thread-safe variant of list_replace()
 *
 * \param	head	cf. list_replace()
 * \param	old		cf. list_replace()
 * \param	new		cf. list_replace()
 * \param	mtx		mutex to lock
 *
 * \return	none
 */
#define list_replace_safe(head, old, new, mtx) \
	LOCK_SECTION(mtx, list_replace(head, old, new))

/**
 * \brief	remove element from list
 *
 * \param	head	pointer to list head
 * \param	el		pointer to element that shall
 * 					be removed
 *
 * \return	none
 */
#define list_rm(head, el){ \
	if((el) != (head))		(el)->prev->next = (el)->next; \
	\
	if((el)->next != 0)		(el)->next->prev = (el)->prev; \
	else					(head)->prev = (el)->prev; \
	\
	if((el) == (head))		(head) = (el)->next; \
}

/**
 * \brief	thread-safe variant of lst_rm()
 *
 * \param	head	cf. list_rm()
 * \param	el	cf. list_rm()
 * \param	mtx		mutex to lock
 */
#define list_rm_safe(head, el, mtx) \
	LOCK_SECTION(mtx, list_rm(head, el))

/**
 * \brief	get the first list element
 *
 * \param	head	pointer to list head
 *
 * \return	pointer to the first list element
 * 			0 if list is empty
 */
#define list_first(head) (head)

/**
 * \brief	thread-safe variant of list_first()
 *
 * \param	head	cf. list_first()
 * \param	mtx		mutex to lock
 *
 * \return	cf. list_first()
 */
#define list_first_safe(head, mtx)({ \
	typeof(head) r; \
	LOCK_SECTION(mtx, r = list_first(head)); \
	r; \
})

/**
 * \brief	get the last list element
 *
 * \param	head	pointer to list head
 *
 * \return	pointer to the last list element
 * 			0 if list is empty
 */
#define list_last(head) ((head) ? (head)->prev : 0)

/**
 * \brief	thread-safe variant of list_last()
 *
 * \param	head	cf. list_last()
 * \param	mtx		mutex to lock
 *
 * \return	cf. list_last()
 */
#define list_last_safe(head, mtx)({ \
	typeof(head) r; \
	LOCK_SECTION(mtx, r = list_last(head)); \
	r; \
})

/**
 * \brief	find an element in the list by key
 *
 * \param	head	pointer to list head
 * \param	member	name of the member that shall be used
 * 					to identify the element
 * \param	value	value the member shall match
 *
 * \return	pointer to list element
 * 			0 if no valid element found
 */
#define list_find(head, member, value) ({ \
	typeof(head) _head = (head); \
\
	for(;_head!=0; _head=_head->next){ \
		if(_head->member == value) \
			break; \
	} \
\
	(_head == 0 || _head->member != value) ? 0 : _head; \
})

/**
 * \brief	thread-safe variant of list_find()
 *
 * \param	head	cf. list_find()
 * \param	member	cf. list_find()
 * \param	value	cf. list_find()
 * \param	mtx		mutex to lock
 *
 * \return	cf. list_find()
 */
#define list_find_safe(head, member, value, mtx)({ \
	typeof(head) r; \
	LOCK_SECTION(mtx, r = list_find(head, member, value)); \
	r; \
})

/**
 * \brief	find an element in the list by key (key is a string)
 *
 * \param	head	pointer to list head
 * \param	member	name of the member that shall be used
 * 					to identify the element
 * \param	str		the string to compare with the member
 *
 * \return	pointer to list element
 * 			0 if no valid element found
 */
#define list_find_str(head, member, str) ({ \
	typeof(head) _head = (head); \
\
	for(;_head!=0; _head=_head->next){ \
		if(strcmp(_head->member, (str)) == 0) \
			break; \
	} \
\
	(_head == 0 || strcmp(_head->member, str) != 0) ? 0: _head; \
})

/**
 * \brief	thread-safe variant of list_find_str()
 *
 * \param	head	cf. list_find_str()
 * \param	member	cf. list_find_str()
 * \param	str		cf. list_find_str()
 * \param	mtx		mutex to lock
 *
 * \return	cf. list_find_str()
 */
#define list_find_str_safe(head, member, str, mtx)({ \
	typeof(head) r; \
	LOCK_SECTION(mtx, r = list_find_str(head, member, str)); \
	r; \
})

/**
 * \brief	same as list_find_str but use strncmp
 */
#define list_find_strn(head, member, str, n) ({ \
	typeof(head) _head = (head); \
\
	for(;_head!=0; _head=_head->next){ \
		if(strncmp(_head->member, (str), (n)) == 0) \
			break; \
	} \
\
	(_head == 0 || strncmp(_head->member, (str), (n)) != 0) ? 0 : _head; \
})

/**
 * \brief	thread-safe variant of list_find_strn()
 *
 * \param	head	cf. list_find_strn()
 * \param	member	cf. list_find_strn()
 * \param	str		cf. list_find_strn()
 * \param	n		cf. list_find_strn()
 * \param	mtx		mutex to lock
 *
 * \return	cf. list_find_strn()
 */
#define list_find_strn_safe(head, member, str, n, mtx)({ \
	typeof(head) r; \
	LOCK_SECTION(mtx, r = list_find_strn(head, member, str, n)); \
	r; \
})

/**
 * \brief	check wether a list is empty
 *
 * \param	head	pointer to list head
 *
 * \return	true	list is empty
 * 			false	list is not empty
 */
#define list_empty(head) (((head) == 0) ? true : false)

/**
 * \brief	thread-safe variant of list_empty()
 *
 * \param	head	cf. list_empty()
 * \param	mtx		mutex to lock
 *
 * \return	cf. list_empty()
 */
#define list_empty_safe(head, mtx)({ \
	int r; \
	LOCK_SECTION(mtx, r = list_empty(head)); \
	r; \
})

/**
 * \brief	iterator macro to loop over
 * 			all list elements
 *
 * \param	head	pointer to list head
 * \param	el	pointer that holds the current element
 *
 * \return	none
 */
#define list_for_each(head, el) el=(head); for(typeof(head) next=((head) == 0 ? 0 : (head)->next); (el)!=0; (el)=(next), next=(next == 0 ? 0 : next->next))


#endif // SYS_LIST_H
