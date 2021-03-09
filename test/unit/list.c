/**
 * Copyright (C) 2016 Jan Nowotsch
 * Author Jan Nowotsch	<jan.nowotsch@gmail.com>
 *
 * Released under the terms of the GNU GPL v2.0
 */



#include <sys/list.h>
#include <sys/types.h>
#include <sys/string.h>
#include <test/test.h>


/* macros */
#define INIT_EL() \
	el0.prev = el0.next = (void*)0x1; \
	el1.prev = el1.next = (void*)0x1; \
	el2.prev = el2.next = (void*)0x1; \
	el3.prev = el3.next = (void*)0x1; \


/* types */
typedef struct tlist_t{
	struct tlist_t *prev,
				   *next;

	int el;
	char const *s_ptr;
	char s_arr[5];
} tlist_t;


/* local/static prototypes */
static void for_each_rm(tlist_t **head);


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
TEST(list_print){
	TEST_LOG("list element addresses\n");
	TEST_LOG("el0 addr: %#x\n", &el0);
	TEST_LOG("el1 addr: %#x\n", &el1);
	TEST_LOG("el2 addr: %#x\n", &el2);
	TEST_LOG("el3 addr: %#x\n", &el3);

	return 0;
}

TEST(list_init){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = &el0;

	list_init(head);

	n += TEST_PTR_EQ(head->prev, head);
	n += TEST_PTR_EQ(head->next, 0x0);

	return -n;
}

TEST(list_empty){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	n += TEST_INT_EQ(list_empty(head), true);

	list_add_head(head, &el0);

	n += TEST_INT_EQ(list_empty(head), false);

	return -n;
}

TEST(list_first_last){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	list_add_head(head, &el3);

	n += TEST_PTR_EQ(list_first(head), &el3);
	n += TEST_PTR_EQ(list_last(head), &el3);

	return -n;
}

TEST(list_add_head){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	list_add_head(head, &el0);
	list_add_head(head, &el1);

	n += TEST_PTR_EQ(head, &el1);
	n += TEST_PTR_EQ(el1.prev, &el0);
	n += TEST_PTR_EQ(el1.next, &el0);
	n += TEST_PTR_EQ(el0.prev, &el1);
	n += TEST_PTR_EQ(el0.next, 0x0);

	return -n;
}

TEST(list_add_tail){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);

	n += TEST_PTR_EQ(head, &el0);
	n += TEST_PTR_EQ(el0.prev, &el1);
	n += TEST_PTR_EQ(el0.next, &el1);
	n += TEST_PTR_EQ(el1.prev, &el0);
	n += TEST_PTR_EQ(el1.next, 0x0);

	return -n;
}

TEST(list_add_in){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);
	list_add_in(&el2, &el0, el0.next);

	n += TEST_PTR_EQ(el1.prev, &el2);
	n += TEST_PTR_EQ(el0.next, &el2);
	n += TEST_PTR_EQ(el2.prev, &el0);
	n += TEST_PTR_EQ(el2.next, &el1);

	return -n;
}

TEST(list_replace){
	unsigned int n;
	tlist_t *head;


	n = 0;

	// replace within empty list
	head = 0x0;
	INIT_EL();

	list_replace(head, &el0, &el1);

	n += TEST_PTR_EQ(head, &el1);
	n += TEST_PTR_EQ(el1.prev, &el1);
	n += TEST_PTR_EQ(el1.next, 0x0);

	// replace head
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_replace(head, &el0, &el1);

	n += TEST_PTR_EQ(head, &el1);
	n += TEST_PTR_EQ(el1.prev, &el1);
	n += TEST_PTR_EQ(el1.next, 0x0);

	// replace tail
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);
	list_replace(head, &el1, &el2);

	n += TEST_PTR_EQ(list_last(head), &el2);
	n += TEST_PTR_EQ(el0.prev, &el2);
	n += TEST_PTR_EQ(el0.next, &el2);
	n += TEST_PTR_EQ(el2.prev, &el0);
	n += TEST_PTR_EQ(el2.next, 0x0);

	// replace middle
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);
	list_add_tail(head, &el2);
	list_replace(head, &el1, &el3);

	n += TEST_PTR_EQ(el0.next, &el3);
	n += TEST_PTR_EQ(el2.prev, &el3);
	n += TEST_PTR_EQ(el3.prev, &el0);
	n += TEST_PTR_EQ(el3.next, &el2);

	return -n;
}

TEST(list_rm){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);
	list_add_tail(head, &el2);

	// remove middle element
	list_rm(head, &el1);

	n += TEST_PTR_EQ(head, &el0);
	n += TEST_PTR_EQ(el0.prev, &el2);
	n += TEST_PTR_EQ(el0.next, &el2);
	n += TEST_PTR_EQ(el2.prev, &el0);
	n += TEST_PTR_EQ(el2.next, 0x0);

	// remove tail element
	list_rm(head, &el2);

	n += TEST_PTR_EQ(head, &el0);
	n += TEST_PTR_EQ(el0.prev, &el0);
	n += TEST_PTR_EQ(el0.next, 0x0);

	// remove head element
	list_rm(head, &el0);

	n += TEST_PTR_EQ(head, 0x0);

	return -n;
}

TEST(list_contains){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);
	list_add_tail(head, &el2);

	n += TEST_INT_EQ(list_contains(head, &el1), true);
	n += TEST_INT_EQ(list_contains(head, &el3), false);

	return -n;
}

TEST(list_find){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;
	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);
	list_add_tail(head, &el2);
	list_add_tail(head, &el3);

	/* list_find() */
	n += TEST_PTR_EQ(list_find(head, el, 2), &el2);
	n += TEST_PTR_EQ(list_find(head, el, 0), &el0);
	n += TEST_PTR_EQ(list_find(head, el, 1), &el1);

	/* list_find_str() with pointer target */
	n += TEST_PTR_EQ(list_find_str(head, s_ptr, "2"), &el2);
	n += TEST_PTR_EQ(list_find_str(head, s_ptr, "0"), &el0);
	n += TEST_PTR_EQ(list_find_str(head, s_ptr, "3"), 0x0);

	/* list_find_strn() with pointer target */
	n += TEST_PTR_EQ(list_find_strn(head, s_ptr, "2", 1), &el2);
	n += TEST_PTR_EQ(list_find_strn(head, s_ptr, "0", 1), &el0);
	n += TEST_PTR_EQ(list_find_strn(head, s_ptr, "3", 1), &el3);

	/* list_find_str() with array target */
	n += TEST_PTR_EQ(list_find_str(head, s_arr, "2"), &el2);
	n += TEST_PTR_EQ(list_find_str(head, s_arr, "0"), &el0);
	n += TEST_PTR_EQ(list_find_str(head, s_arr, "3"), 0x0);

	/* list_find_strn() with array target */
	n += TEST_PTR_EQ(list_find_strn(head, s_arr, "2", 1), &el2);
	n += TEST_PTR_EQ(list_find_strn(head, s_arr, "0", 1), &el0);
	n += TEST_PTR_EQ(list_find_strn(head, s_arr, "3", 1), &el3);

	/* error cases */
	n += TEST_PTR_EQ(list_find(head, el, 4), 0x0);
	n += TEST_PTR_EQ(list_find((tlist_t*)0x0, el, 4), 0x0);
	n += TEST_PTR_EQ(list_find_str((tlist_t*)0x0, s_arr, "4"), 0x0);

	return -n;
}

TEST(list_for_each){
	unsigned int n;
	tlist_t *head;


	n = 0;
	head = 0x0;

	for_each_rm(&head);

	INIT_EL();

	list_add_tail(head, &el0);
	list_add_tail(head, &el1);
	list_add_tail(head, &el2);
	list_add_tail(head, &el3);

	for_each_rm(&head);

	n += TEST_INT_EQ(list_empty(head), true);

	return -n;
}

static void for_each_rm(tlist_t **head){
	tlist_t *el;


	list_for_each(*head, el)
		list_rm(*head, el);
}
