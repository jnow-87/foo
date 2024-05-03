/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/queue.h>
#include <sys/types.h>
#include <sys/string.h>
#include <test/test.h>


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
TEST(queue_print){
	TEST_LOG("list element addresses\n");
	TEST_LOG("el0 addr: %#x\n", &el0);
	TEST_LOG("el1 addr: %#x\n", &el1);
	TEST_LOG("el2 addr: %#x\n", &el2);
	TEST_LOG("el3 addr: %#x\n", &el3);

	return 0;
}

TEST(queue_init){
	int r = 0;
	queue_t *head = &el0;
	queue_t *tail;


	queue_init(head, tail);

	r |= TEST_PTR_EQ(head->next, 0);
	r |= TEST_PTR_EQ(tail, head);

	return -r;
}

TEST(queue_enqueue){
	int r = 0;
	queue_t *head = 0x0,
			*tail = 0x0;


	INIT_EL();

	// enqueue to empty queue
	queue_enqueue(head, tail, &el0);

	r |= TEST_PTR_EQ(head, &el0);
	r |= TEST_PTR_EQ(tail, &el0);
	r |= TEST_PTR_EQ(el0.next, 0);

	// enqueue in non-empty queue
	queue_enqueue(head, tail, &el1);

	r |= TEST_PTR_EQ(head, &el0);
	r |= TEST_PTR_EQ(tail, &el1);
	r |= TEST_PTR_EQ(el0.next, &el1);
	r |= TEST_PTR_EQ(el1.next, 0);

	return -r;
}

TEST(queue_dequeue){
	int r = 0;
	queue_t *head = 0x0,
			*tail = 0x0;
	queue_t *el;


	INIT_EL();

	// prepare queue
	queue_enqueue(head, tail, &el0);
	queue_enqueue(head, tail, &el1);

	// dequeue from non-empty queue
	el = dequeue(&head, &tail);

	r |= TEST_PTR_EQ(head, &el1);
	r |= TEST_PTR_EQ(tail, &el1);
	r |= TEST_PTR_EQ(el, &el0);

	// dequeue last element
	el = dequeue(&head, &tail);

	r |= TEST_PTR_EQ(head, 0);
	r |= TEST_PTR_EQ(tail, 0);
	r |= TEST_PTR_EQ(el, &el1);

	// dequeue from empty empty queue
	el = dequeue(&head, &tail);

	r |= TEST_PTR_EQ(head, 0);
	r |= TEST_PTR_EQ(tail, 0);
	r |= TEST_PTR_EQ(el, 0);

	return -r;
}

TEST(queue_empty){
	int r = 0;
	queue_t *head = 0x0,
			*tail = 0x0;


	INIT_EL();

	r |= TEST_INT_EQ(queue_empty(head), true);

	queue_enqueue(head, tail, &el0);

	r |= TEST_INT_EQ(queue_empty(head), false);

	return -r;
}

TEST(queue_head){
	int r = 0;
	queue_t *head = 0x0,
			*tail = 0x0;


	INIT_EL();

	queue_enqueue(head, tail, &el3);

	r |= TEST_PTR_EQ(queue_head(head), &el3);

	return -r;
}

static queue_t *dequeue(queue_t **head, queue_t **tail){
	return queue_dequeue(*head, *tail);
}
