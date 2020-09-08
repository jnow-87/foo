/**
 * Copyright (C) 2018 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/stack.h>
#include <sys/types.h>
#include <sys/string.h>
#include <testing/testcase.h>


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
TEST(stack_print, "stack print"){
	TEST_LOG("list element addresses\n");
	TEST_LOG("el0 addr: %#x\n", &el0);
	TEST_LOG("el1 addr: %#x\n", &el1);
	TEST_LOG("el2 addr: %#x\n", &el2);
	TEST_LOG("el3 addr: %#x\n", &el3);

	return 0;
}

TEST(stack_init, "stack init"){
	unsigned int n;
	stack_t *top;


	n = 0;
	top = &el0;

	stack_init(top);

	n += CHECK_PTR(top->next, 0);

	return -n;
}

TEST(stack_push, "stack push"){
	unsigned int n;
	stack_t *top;


	n = 0;
	top = 0;
	INIT_EL();

	// push to empty stack
	stack_push(top, &el0);

	n += CHECK_PTR(top, &el0);
	n += CHECK_PTR(el0.next, 0);

	// push to non-empty stack
	stack_push(top, &el1);

	n += CHECK_PTR(top, &el1);
	n += CHECK_PTR(el1.next, &el0);
	n += CHECK_PTR(el0.next, 0);

	return -n;
}

TEST(stack_pop, "stack pop"){
	unsigned int n;
	stack_t *top,
			*el;


	n = 0;
	top = 0;
	INIT_EL();

	// prepare
	stack_push(top, &el0);
	stack_push(top, &el1);

	// pop from non-empty stack
	el = pop(&top);

	n += CHECK_PTR(top, &el0);
	n += CHECK_PTR(el, &el1);

	// pop last element
	el = pop(&top);

	n += CHECK_PTR(top, 0);
	n += CHECK_PTR(el, &el0);

	// pop from empty stack
	el = pop(&top);

	n += CHECK_PTR(top, 0);
	n += CHECK_PTR(el, 0);

	return -n;
}

static stack_t *pop(stack_t **top){
	return stack_pop(*top);
}
