#ifndef SYS_LIST_H
#define SYS_LIST_H


/**
 * version of the list macros that use the head element as functional
 * part of the list, i.e. it also stores data
 * head->prev points to the last element of the list, while the list
 * is delimited by 0 with head->prev->next = 0
 */


/**
 * \brief	macros to handle a double-linked list
 * 			they can be used with any structure that
 * 			contains a next and prev pointer with
 * 			type of the struct
 *
 * 			list head needs to be initialized with
 * 			list_init() this element is not used to
 * 			store any data
 */

/**
 * \brief	initialize the head of a list
 *
 * \param	head	pointer to list head
 *
 * \return	none
 */
#define list_init(head){ \
	if((head) != 0){ \
		(head)->next = 0; \
		(head)->prev = (head); \
	} \
}

/**
 * \brief	add an element at head of the list
 *
 * \param	head	pointer to list head
 * \param	el		pointer to element to insert
 *
 * \return	new head
 */
#define list_add_head(head, el){ \
	typeof(el) _el = (el); \
	\
	\
	(_el)->next = *(head); \
	(_el)->prev = (*(head) == 0 ? (_el) : (*(head))->prev); \
	if(*(head) != 0) \
		(*(head))->prev = _el; \
	\
	*(head) = _el; \
}

/**
 * \brief	add an element at the end of the list
 *
 * \param	head	pointer to list head
 * \param	el		pointer to element to insert
 *
 * \return	none
 */
#define list_add_tail(head, el){ \
	typeof(el) _el = el; \
	\
	\
	(_el)->next = 0; \
	if(*(head) == 0){ \
		*(head) = _el; \
		(_el)->prev = _el; \
	} \
	else{ \
		(_el)->prev = (*(head))->prev; \
		(*(head))->prev->next = _el; \
		(*(head))->prev = _el; \
	} \
}

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
	typeof(new) _new = new; \
	\
	\
	if(*(head) == 0){ \
		*(head) = _new; \
		(_new)->prev = _new; \
		(_new)->next = 0; \
	} \
	else{ \
		(_new)->prev = ((*(head))->next == 0) ? _new : (old)->prev; \
		(_new)->next = (old)->next; \
		\
		if((old)->next)	(old)->next->prev = _new; \
		else			(*(head))->prev = _new; \
		\
		if(old != *(head))	(old)->prev->next = _new; \
		else				*(head) = _new; \
	} \
}

/**
 * \brief	remove element from list
 *
 * \param	head	pointer to list head
 * \param	entry	pointer to element that shall
 * 					be removed
 *
 * \return new head (head is updated of entry == head)
 */
#define list_rm(head, entry){ \
	if((entry) != (*(head)))	(entry)->prev->next = (entry)->next; \
	\
	if((entry)->next != 0)		(entry)->next->prev = (entry)->prev; \
	else						(*(head))->prev = (entry)->prev; \
	\
	if((entry) == (*(head)))	*(head) = (entry)->next; \
}

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
 * \brief	get the last list element
 *
 * \param	head	pointer to list head
 *
 * \return	pointer to the last list element
 * 			0 if list is empty
 */
#define list_last(head) ((head) ? (head)->prev : 0)

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
	(_head == 0 || strncmp(_head->member, str, (n)) != 0) ? 0 : _head; \
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
 * \brief	iterator macro to loop over
 * 			all list elements
 *
 * \param	head	pointer to list head
 * \param	entry	pointer that holds the current element
 *
 * \return none
 */
#define list_for_each(head, entry) entry=(head); for(typeof(head) next=((head) == 0 ? 0 : (head)->next); (entry)!=0; entry=(next), next=(next == 0 ? 0 : next->next))


#endif // SYS_LIST_H
