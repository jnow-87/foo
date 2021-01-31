/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/queue.h>
#include <sys/types.h>
#include <sys/string.h>
#include <testcase.h>


/* macros */
#define INIT_EL() \
	el0.next = (void*)0x1; \
	el1.next = (void*)0x1; \
	el2.next = (void*)0x1; \
	el3.next = (void*)0x1; \


/* types */
typedef struct queue_t{
	struct queue_t *next;

	int el;
	char s[5];
} queue_t;


/* local/static prototypes */
static queue_t *dequeue(queue_t **head, queue_t **tail);


/* static variables */
static queue_t el0 = { .el = 0, .s = "0" },
			   el1 = { .el = 1, .s = "1" },
			   el2 = { .el = 2, .s = "2" },
			   el3 = { .el = 3, .s = "3xx" };


/* local functions */
TEST(queue_print, "queue print"){
	TEST_LOG("list element addresses\n");
	TEST_LOG("el0 addr: %#x\n", &el0);
	TEST_LOG("el1 addr: %#x\n", &el1);
	TEST_LOG("el2 addr: %#x\n", &el2);
	TEST_LOG("el3 addr: %#x\n", &el3);

	return 0;
}

TEST(queue_init, "queue init"){
	unsigned int n;
	queue_t *head,
			*tail;


	n = 0;
	head = &el0;

	queue_init(head, tail);

	n += CHECK_PTR(head->next, 0);
	n += CHECK_PTR(tail, head);

	return -n;
}

TEST(queue_enqueue, "queue enqueue"){
	unsigned int n;
	queue_t *head,
			*tail;


	n = 0;
	head = 0;
	tail = 0;
	INIT_EL();

	// enqueue to empty queue
	queue_enqueue(head, tail, &el0);

	n += CHECK_PTR(head, &el0);
	n += CHECK_PTR(tail, &el0);
	n += CHECK_PTR(el0.next, 0);

	// enqueue in non-empty queue
	queue_enqueue(head, tail, &el1);

	n += CHECK_PTR(head, &el0);
	n += CHECK_PTR(tail, &el1);
	n += CHECK_PTR(el0.next, &el1);
	n += CHECK_PTR(el1.next, 0);

	return -n;
}

TEST(queue_dequeue, "queue dequeue"){
	unsigned int n;
	queue_t *head,
			*tail,
			*el;


	n = 0;
	head = 0;
	tail = 0;
	INIT_EL();

	// prepare queue
	queue_enqueue(head, tail, &el0);
	queue_enqueue(head, tail, &el1);

	// dequeue from non-empty queue
	el = dequeue(&head, &tail);

	n += CHECK_PTR(head, &el1);
	n += CHECK_PTR(tail, &el1);
	n += CHECK_PTR(el, &el0);

	// dequeue last element
	el = dequeue(&head, &tail);

	n += CHECK_PTR(head, 0);
	n += CHECK_PTR(tail, 0);
	n += CHECK_PTR(el, &el1);

	// dequeue from empty empty queue
	el = dequeue(&head, &tail);

	n += CHECK_PTR(head, 0);
	n += CHECK_PTR(tail, 0);
	n += CHECK_PTR(el, 0);

	return -n;
}

static queue_t *dequeue(queue_t **head, queue_t **tail){
	return queue_dequeue(*head, *tail);
}
