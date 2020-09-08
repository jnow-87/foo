/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/list.h>
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
typedef struct tlist_t{
	struct tlist_t *next;

	int el;
	char const *s_ptr;
	char s_arr[5];
} tlist_t;


/* static variables */
static char const el_names[][5] = {
	"0",
	"1",
	"2",
	"3xx",
};

static tlist_t el0 = { .el = 0, .s_ptr = el_names[0], .s_arr = "0" },
			   el1 = { .el = 1, .s_ptr = el_names[1], .s_arr = "1" },
			   el2 = { .el = 2, .s_ptr = el_names[2], .s_arr = "2" },
			   el3 = { .el = 3, .s_ptr = el_names[3], .s_arr = "3xx" };


/* local functions */
TEST(list1_init, "list1 init"){
	unsigned int n;
	tlist_t *head,
			*tail;


	n = 0;
	head = &el0;

	list1_init(head, tail);

	n += CHECK_PTR(head->next, 0x0);

	return -n;
}

TEST(list2_add_head, "list1 add head"){
	unsigned int n;
	tlist_t *head,
			*tail;


	n = 0;
	head = 0x0;
	tail = 0x0;
	INIT_EL();

	list1_add_head(head, tail, &el0);
	list1_add_head(head, tail, &el1);

	n += CHECK_PTR(head, &el1);
	n += CHECK_PTR(el1.next, &el0);
	n += CHECK_PTR(el0.next, 0x0);

	return -n;
}

TEST(list1_add_tail, "list1 add tail"){
	unsigned int n;
	tlist_t *head,
			*tail;


	n = 0;
	head = 0x0;
	tail = 0x0;
	INIT_EL();

	list1_add_tail(head, tail, &el0);
	list1_add_tail(head, tail, &el1);

	n += CHECK_PTR(head, &el0);
	n += CHECK_PTR(el0.next, &el1);
	n += CHECK_PTR(el1.next, 0x0);

	return -n;
}

TEST(list1_rm, "list1 rm"){
	unsigned int n;
	tlist_t *head,
			*tail;


	n = 0;
	head = 0x0;
	tail = 0x0;
	INIT_EL();

	list1_add_tail(head, tail, &el0);
	list1_add_tail(head, tail, &el1);
	list1_add_tail(head, tail, &el2);

	// remove middle element
	list1_rm_head(head, tail);

	n += CHECK_PTR(head, &el1);
	n += CHECK_PTR(el1.next, &el2);
	n += CHECK_PTR(el2.next, 0x0);

	// remove tail element
	list1_rm_head(head, tail);

	n += CHECK_PTR(head, &el2);
	n += CHECK_PTR(el2.next, 0x0);

	// remove head element
	list1_rm_head(head, tail);

	n += CHECK_PTR(head, 0x0);

	return -n;
}
