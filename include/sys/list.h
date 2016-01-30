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
#define list_add_head(head, _el){ \
	typeof(_el) el = (_el); \
	\
	\
	(el)->next = *(head); \
	(el)->prev = (*(head) == 0 ? (el) : (*(head))->prev); \
	if(*(head) != 0) \
		(*(head))->prev = el; \
	\
	*(head) = el; \
}

/**
 * \brief	add an element at the end of the list
 *
 * \param	head	pointer to list head
 * \param	el		poiinter to element to insert
 *
 * \return	none
 */
#define list_add_tail(head, _el){ \
	typeof(_el) el = _el; \
	\
	\
	(el)->next = 0; \
	if(*(head) == 0){ \
		*(head) = el; \
		(el)->prev = el; \
	} \
	else{ \
		(el)->prev = (*(head))->prev; \
		(*(head))->prev->next = el; \
		(*(head))->prev = el; \
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
	if((entry) != (*(head))) (entry)->prev->next = (entry)->next; \
	if((entry)->next != 0) (entry)->next->prev = (entry)->prev; \
	else (*(head))->prev = (entry)->prev; \
	\
	*(head) = (entry) == (*(head)) ? (entry)->next : *(head); \
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
	typeof(head) tmp = (head); \
\
	for(;tmp!=0; tmp=tmp->next){ \
		if(tmp->member == value) \
			break; \
	} \
\
	(tmp == 0 || tmp->member != value) ? 0 : tmp; \
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
	typeof(head) tmp = (head); \
\
	for(;tmp!=0; tmp=tmp->next){ \
		if(strcmp(tmp->member, (str)) == 0) \
			break; \
	} \
\
	(tmp == 0 || strcmp(tmp->member, str) != 0) ? 0: tmp; \
})

/**
 * \brief	same as list_find_str but use strncmp
 */
#define list_find_strn(head, member, str, n) ({ \
	typeof(head) tmp = (head); \
\
	for(;tmp!=0; tmp=tmp->next){ \
		if(strncmp(tmp->member, (str), (n)) == 0) \
			break; \
	} \
\
	(tmp == 0 || strncmp(tmp->member, str, (n)) != 0) ? 0 : tmp; \
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
#define list_for_each(head, entry) entry=(head); for(typeof(header) next=((head) == 0 ? 0 : (head)->next); (entry)!=0; entry=(next), next=(next == 0 ? 0 : next->next))


#endif // SYS_LIST_H
