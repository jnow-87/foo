/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/string.h>
#include <sys/list.h>


/* global functions */
void _list2_init(list2_t *head){
	head->prev = head;
	head->next = 0x0;
}

void _list2_add_head(list2_t **head, list2_t *el){
	el->next = *head;
	el->prev = (*head == 0x0) ? el : (*head)->prev;

	if(*head != 0x0)
		(*head)->prev = el;

	*head = el;
}

void _list2_add_tail(list2_t **head, list2_t *el){
	if(*head != 0x0){
		el->prev = (*head)->prev;
		(*head)->prev->next = el;
	}
	else
		*head = el;

	el->next = 0x0;
	(*head)->prev = el;
}

void _list2_add_in(list2_t *el, list2_t *front, list2_t *back){
	el->prev = front;
	el->next = back;
	front->next = el;
	back->prev = el;
}

void _list2_replace(list2_t **head, list2_t *old, list2_t *new){
	if(*head == 0x0){
		*head = new;
		new->prev = new;
		new->next = 0x0;
	}
	else{
		new->prev = ((*head)->next == 0x0) ? new : old->prev;
		new->next = old->next;

		if(old->next)		old->next->prev = new;
		else				(*head)->prev = new;

		if(old != *head)	old->prev->next = new;
		else				*head = new;
	}
}

void _list2_rm(list2_t **head, list2_t *el){
	if(el != *head)
		el->prev->next = el->next;

	if(el->next != 0x0)	el->next->prev = el->prev;
	else				(*head)->prev = el->prev;

	if(el == *head)		*head = el->next;
}

list2_t *_list2_last(list2_t *head){
	return (head != 0x0) ? head->prev : 0x0;
}

bool _list2_contains(list2_t *head, list2_t *el){
	for(;head!=0x0; head=head->next){
		if(head == el)
			break;
	}

	return head == el;
}

list2_t *_list2_find(list2_t *head, size_t mem_offset, void const *ref, size_t n){
	list2_t *el;


	list_for_each(head, el){
		if(memcmp(((char*)el) + mem_offset, ref, n) == 0)
			return el;
	}

	return 0x0;
}

list2_t *_list2_find_str(list2_t *head, size_t mem_offset, char const *ref, size_t n, bool is_array){
	list2_t *el;
	char *s;


	list_for_each(head, el){
		if(is_array)	s = ((char*)el) + mem_offset;
		else			s = *((char**)(((char*)el) + mem_offset));

		if(n == 0){
			if(strcmp(s, ref) == 0)
				return el;
		}
		else if(strncmp(s, ref, n) == 0)
			return el;
	}

	return 0x0;
}
