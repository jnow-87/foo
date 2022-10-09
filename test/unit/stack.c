/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/stack.h>
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
typedef struct stack_t{
	struct stack_t *next;

	int el;
	char s[5];
} stack_t;


/* local/static prototypes */
static stack_t *pop(stack_t **top);


/* static variables */
static stack_t el0 = { .el = 0, .s = "0" },
			   el1 = { .el = 1, .s = "1" },
			   el2 = { .el = 2, .s = "2" },
			   el3 = { .el = 3, .s = "3xx" };


/* local functions */
TEST(stack_print){
	TEST_LOG("list element addresses\n");
	TEST_LOG("el0 addr: %#x\n", &el0);
	TEST_LOG("el1 addr: %#x\n", &el1);
	TEST_LOG("el2 addr: %#x\n", &el2);
	TEST_LOG("el3 addr: %#x\n", &el3);

	return 0;
}

TEST(stack_init){
	unsigned int n = 0;
	stack_t *top = &el0;


	stack_init(top);

	n += TEST_PTR_EQ(top->next, 0);

	return -n;
}

TEST(stack_top){
	unsigned int n = 0;
	stack_t *top = 0x0;


	INIT_EL();

	n += TEST_PTR_EQ(stack_top(top), 0x0);

	stack_push(top, &el0);
	n += TEST_PTR_EQ(stack_top(top), &el0);

	stack_push(top, &el1);
	n += TEST_PTR_EQ(stack_top(top), &el1);

	(void)pop(&top);
	n += TEST_PTR_EQ(stack_top(top), &el0);

	(void)pop(&top);
	n += TEST_PTR_EQ(stack_top(top), 0x0);

	return -n;
}

TEST(stack_push){
	unsigned int n = 0;
	stack_t *top = 0x0;


	INIT_EL();

	// push to empty stack
	stack_push(top, &el0);

	n += TEST_PTR_EQ(top, &el0);
	n += TEST_PTR_EQ(el0.next, 0);

	// push to non-empty stack
	stack_push(top, &el1);

	n += TEST_PTR_EQ(top, &el1);
	n += TEST_PTR_EQ(el1.next, &el0);
	n += TEST_PTR_EQ(el0.next, 0);

	return -n;
}

TEST(stack_pop){
	unsigned int n = 0;
	stack_t *top = 0x0,
			*el;


	INIT_EL();

	// prepare
	stack_push(top, &el0);
	stack_push(top, &el1);

	// pop from non-empty stack
	el = pop(&top);

	n += TEST_PTR_EQ(top, &el0);
	n += TEST_PTR_EQ(el, &el1);

	// pop last element
	el = pop(&top);

	n += TEST_PTR_EQ(top, 0);
	n += TEST_PTR_EQ(el, &el0);

	// pop from empty stack
	el = pop(&top);

	n += TEST_PTR_EQ(top, 0);
	n += TEST_PTR_EQ(el, 0);

	return -n;
}

static stack_t *pop(stack_t **top){
	return stack_pop(*top);
}
