/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/list.h>


/* global functions */
void _list1_init(list1_t *head, list1_t **tail){
	head->next = 0x0;
	*tail = head;
}

void _list1_add_head(list1_t **head, list1_t **tail, list1_t *el){
	el->next = *head;
	*head = el;

	if(*tail == 0x0)
		*tail = *head;
}

void _list1_add_tail(list1_t **head, list1_t **tail, list1_t *el){
	el->next = 0x0;

	if(*tail != 0x0)
		(*tail)->next = el;

	*tail = el;

	if(*head == 0x0)
		*head = el;
}

void _list1_rm_head(list1_t **head, list1_t **tail){
	if(*tail == *head)
		*tail = 0x0;

	*head = (*head)->next;
}
